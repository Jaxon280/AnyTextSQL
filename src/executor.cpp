#include "executor.hpp"

using namespace vlex;

Executor::Executor() {}
Executor::Executor(VectFA *vfa, QueryContext *query, SIZE_TYPE _start) {
    setVFA(vfa, _start);
    setQuery(query);
}

Executor::~Executor() {}

void Executor::setVFA(VectFA *vfa, SIZE_TYPE _start) {
    ctx.currentState = INIT_STATE;
    ctx.tokenStartIndex = _start, ctx.recentAcceptIndex = 0,
    ctx.recentAcceptState = 0;
    i = _start;

    std::vector<Qlabel> &qlabels = vfa->getQlabels();
    int stateSize = qlabels.size();
    for (int q = 0; q < stateSize; q++) {
        if (qlabels[q].isAccept) {
            acceptStates.insert(q);
        }
    }

    charTable = new ST_TYPE *[stateSize];
    kindTable = new SIMDKind[stateSize];
    rTable = new ST_TYPE[stateSize];

    for (int s = 0; s < stateSize; s++) {
        charTable[s] = new ST_TYPE[256];

        kindTable[s] = qlabels[s].kind;
        if (qlabels[s].kind == ORDERED) {
            rTable[s] = qlabels[s].delta->rTable[0];
            for (int j = 0; j < 256; j++) {
                charTable[s][j] = 0;
            }
        } else {
            rTable[s] = 0;
            for (int j = 0; j < 256; j++) {
                charTable[s][j] = qlabels[s].delta->charTable[j];
            }
        }
    }

    anyStartTable = new int[stateSize]();
    anyEndTable = new int[stateSize]();
    charStartTable = new int[stateSize]();
    charEndTable = new int[stateSize]();
    std::vector<DFA::SubMatchStates> &subMatchStates = vfa->getSubMatches();
    subMatchSize = subMatchStates.size();
    int id = 1;
    for (DFA::SubMatchStates &sms : subMatchStates) {
        if (sms.isAnyStart) {
            anyStartTable[sms.startState] = id;
        } else {
            charStartTable[sms.startState] = id;
        }
        if (sms.isAnyEnd) {
            anyEndTable[sms.endState] = id;
        } else {
            charEndTable[sms.endState] = id;
        }
        id++;
    }

    end = new SubMatchNode;
    if (id > 1) {
        subMatches = new SubMatch[id - 1];
    } else {
        subMatches = nullptr;
    }

    SIMDDatas = new SIMD_TEXTTYPE[stateSize];
    SIMDSizes = new int[stateSize];
    int iter = 0;
    for (Qlabel &it : qlabels) {
        if (it.kind == ORDERED || it.kind == ANY || it.kind == RANGES) {
            DATA_TYPE strdata[16];
            for (int k = 0; k < 16; k++) {
                if (k < it.delta->str.size()) {
                    strdata[k] = (DATA_TYPE)it.delta->str[k];
                } else {
                    strdata[k] = 0;
                }
            }

            SIMDDatas[iter] =
                _mm_loadu_si128(reinterpret_cast<SIMD_TEXTTYPE *>(strdata));
            SIMDSizes[iter] = (int)it.delta->str.size();
        }
        iter++;
    }
}

void Executor::setQuery(QueryContext *query) {
    keyTypes = new Type[subMatchSize];
    for (int i = 0; i < subMatchSize; i++) {
        keyTypes[i] = query->getKeyTypes()[i];
    }
    // todo: where to get key type information?

    indexes = new SIZE_TYPE *[subMatchSize];
    sizes = new SIZE_TYPE *[subMatchSize];
    for (int k = 0; k < subMatchSize; k++) {
        indexes[k] = new (std::align_val_t{VECEX_SIZE}) SIZE_TYPE[VECEX_BYTE];
        sizes[k] = new (std::align_val_t{VECEX_SIZE}) SIZE_TYPE[VECEX_BYTE];
    }

    QueryContext::Selection &sel = query->getSelection();
    numPreds = sel.numPreds;
    if (sel.isCNF) {
        predANDsize = sel.preds.size();
        predORsize = new int[predANDsize];
        for (int ai = 0; ai < predANDsize; ai++) {
            predORsize[ai] = sel.preds[ai].size();
        }
        preds = new int *[predANDsize];
        for (int ai = 0; ai < predANDsize; ai++) {
            preds[ai] = new int[predORsize[ai]];
            for (int oi = 0; oi < predORsize[ai]; oi++) {
                preds[ai][oi] = sel.preds[ai][oi];
            }
        }
    } else {
        // todo: add to DNF
    }

    predMasks = new uint8_t *[numPreds];
    for (int k = 0; k < numPreds; k++) {
        predMasks[k] = new (std::align_val_t{VECEX_SIZE}) uint8_t[VECEX_BYTE4];
    }
    mask = new (std::align_val_t{VECEX_SIZE}) uint16_t[VECEX_BYTE16];

    predTypes = new Type[numPreds];
    predTrees = new QueryContext::OpTree[numPreds];
    for (int k = 0; k < numPreds; k++) {
        predTypes[k] = sel.predTypes[k];
        predTrees[k] = sel.predTrees[k];
    }

    bufArray = new data64 *[subMatchSize];
    for (int k = 0; k < subMatchSize; k++) {
        bufArray[k] = new (std::align_val_t{VECEX_SIZE}) data64[VECEX_BYTE];
    }

    tupleIds = new (std::align_val_t{VECEX_SIZE}) uint32_t[VECEX_BYTE];
    for (int i = 0; i < VECEX_BYTE; i++) {
        tupleIds[i] = i;
    }
    selectionVector = new uint32_t[VECEX_BYTE];

    QueryContext::Projection proj = query->getProjection();
    numProjKeys = proj.columns.size();

    aggContext = query->getAggregation();

    aggVSize = aggContext->valueKeys.size();
    if (aggContext->keys.size() == 1) {
        aggMap = new AggregationValueMap(aggContext->valueKeys.size());
        aggCountMap = new AggregationCountMap(aggContext->valueKeys.size());
    } else if (aggContext->keys.size() == 0) {
        agg = new data64[aggVSize];
        aggCount = new int[aggVSize];
        for (int vk = 0; vk < aggVSize; vk++) {
            AggFuncType ftype = aggContext->valueKeys[vk].ftype;
            if (aggContext->valueKeys[vk].ktype == DOUBLE) {
                if (ftype == MIN) {
                    agg[vk].d = DBL_MAX;
                } else if (ftype == MAX) {
                    agg[vk].d = -DBL_MAX;
                } else {
                    agg[vk].d = 0.0;
                }
            } else if (aggContext->valueKeys[vk].ktype == INT) {
                if (ftype == MIN) {
                    agg[vk].i = INT64_MAX;
                } else if (ftype == MAX) {
                    agg[vk].i = INT64_MIN;
                } else {
                    agg[vk].d = 0;
                }
            }
            aggCount[vk] = 0;
        }
    }

    limit = query->getLimit();
}

inline void Executor::cmpestri_ord(ST_TYPE cur_state) {
loop:
    if (i >= size) return;
    SIMD_TEXTTYPE text =
        _mm_loadu_si128(reinterpret_cast<SIMD_TEXTTYPE *>(&data[i]));
    int r = _mm_cmpestri(SIMDDatas[cur_state], SIMDSizes[cur_state], text, 16,
                         _SIDD_CMP_EQUAL_ORDERED);
    if (r == 16) {
        i += 16;
        goto loop;
    }
    i += r;
    if (r > 16 - SIMDSizes[cur_state]) goto loop;
    i += SIMDSizes[cur_state];
    if (ctx.currentState == INIT_STATE) {
        resetContext();
    }
    ctx.currentState = rTable[cur_state];
}

inline void Executor::cmpestri_any(ST_TYPE cur_state) {
    if (anyStartTable[cur_state] > 0) {
        startSubMatch(anyStartTable[cur_state]);
    }
loop:
    if (i >= size) return;
    SIMD_TEXTTYPE text =
        _mm_loadu_si128(reinterpret_cast<SIMD_TEXTTYPE *>(&data[i]));
    int r = _mm_cmpestri(SIMDDatas[cur_state], SIMDSizes[cur_state], text, 16,
                         _SIDD_CMP_EQUAL_ANY);
    if (r == 16) {
        i += 16;
        goto loop;
    }
    i += r;
    if (anyEndTable[cur_state] > 0) {
        endSubMatch(anyEndTable[cur_state]);
    }
    if (ctx.currentState == INIT_STATE) {
        resetContext();
    }

    if (charStartTable[ctx.currentState] > 0) {
        startSubMatch(charStartTable[ctx.currentState]);
    }
    ctx.currentState = charTable[cur_state][(int)data[i++]];
    if (charEndTable[ctx.currentState] > 0) {
        endSubMatch(charEndTable[ctx.currentState]);
    }
}

inline void Executor::cmpestri_ranges(ST_TYPE cur_state) {
    if (anyStartTable[cur_state] > 0) {
        startSubMatch(anyStartTable[cur_state]);
    }
loop:
    if (i >= size) return;
    SIMD_TEXTTYPE text =
        _mm_loadu_si128(reinterpret_cast<SIMD_TEXTTYPE *>(&data[i]));
    int r = _mm_cmpestri(SIMDDatas[cur_state], SIMDSizes[cur_state], text, 16,
                         _SIDD_CMP_RANGES);
    if (r == 16) {
        i += 16;
        goto loop;
    }
    i += r;
    if (anyEndTable[cur_state] > 0) {
        endSubMatch(anyEndTable[cur_state]);
    }
    if (ctx.currentState == INIT_STATE) {
        resetContext();
    }

    if (charStartTable[ctx.currentState] > 0) {
        startSubMatch(charStartTable[ctx.currentState]);
    }
    ctx.currentState = charTable[cur_state][(int)data[i++]];
    if (charEndTable[ctx.currentState] > 0) {
        endSubMatch(charEndTable[ctx.currentState]);
    }
}

inline void Executor::startSubMatch(int id) {
    if (end->id > 0) {
        while (!startStack.empty()) {
            if (startStack.top().id == end->id) {
                subMatches[end->id - 1].start = startStack.top().index;
                subMatches[end->id - 1].end = end->index;
                break;
            }
            startStack.pop();
        }
        end->id = 0;
    }
    startStack.push({id, i});
}

inline void Executor::endSubMatch(int id) {
    if (end->id > 0 && end->id != id) {
        while (!startStack.empty()) {
            if (startStack.top().id == end->id) {
                subMatches[end->id - 1].start = startStack.top().index;
                subMatches[end->id - 1].end = end->index;
                startStack.pop();
                break;
            }
            startStack.pop();
        }
    }
    end->id = id;
    end->index = i;
}

inline void Executor::resetContext() {
    ctx.recentAcceptState = 0, ctx.recentAcceptIndex = 0;
    ctx.tokenStartIndex = i;
    end->id = 0;
    while (!startStack.empty()) {
        startStack.pop();
    }
}

void Executor::vmaterialize() {
    for (int s = 0; s < subMatchSize; s++) {
        // todo: not materialization for key only used in projection/aggregation
        SIMD_256iTYPE base = _mm256_set1_epi64x((int64_t)data);
        switch (keyTypes[s]) {
            case DOUBLE:
                for (int ti = 0; ti < VECEX_BYTE; ti++) {
                    DATA_TYPE buf[sizes[s][ti] + 1];
                    buf[sizes[s][ti]] = (DATA_TYPE)0;
                    memcpy(buf, &data[indexes[s][ti]], sizes[s][ti]);
                    double d = atof((char *)buf);
                    bufArray[s][ti].d = d;
                }
                break;
            case INT:
                for (int ti = 0; ti < VECEX_BYTE; ti++) {
                    DATA_TYPE buf[sizes[s][ti] + 1];
                    buf[sizes[s][ti]] = (DATA_TYPE)0;
                    memcpy(buf, &data[indexes[s][ti]], sizes[s][ti]);
                    int64_t n = (int64_t)atoi((char *)buf);
                    bufArray[s][ti].i = n;
                }
                break;
            case TEXT:
                for (int ti = 0; ti < VECEX_BYTE; ti += 4) {
                    SIMD_128iTYPE offsets = _mm_load_si128(
                        reinterpret_cast<SIMD_128iTYPE *>(&indexes[s][ti]));
                    SIMD_256iTYPE adrs =
                        _mm256_add_epi64(base, _mm256_cvtepi32_epi64(offsets));
                    _mm256_store_si256(
                        reinterpret_cast<SIMD_256iTYPE *>(&bufArray[s][ti]),
                        adrs);
                }
                break;
            default:
                break;
        }
    }
}

void Executor::vevalPreds() {
    for (int sk = 0; sk < numPreds; sk++) {
        if (predTypes[sk] == DOUBLE) {
            PredVEvaluator<double> prede(predMasks[sk]);
            prede.evaluate(&predTrees[sk], bufArray);
        } else if (predTypes[sk] == INT) {
            PredVEvaluator<int64_t> prede(predMasks[sk]);
            prede.evaluate(&predTrees[sk], bufArray);
        } else if (predTypes[sk] == TEXT) {
            PredVEvaluator<DATA_TYPE *> prede(predMasks[sk]);
            prede.evaluateText(&predTrees[sk], bufArray, sizes);
        }
    }
}

void Executor::veval() {
    int svi = 0;
    if (numPreds == 0) {
        for (int i = 0; i < VECEX_BYTE; i++) {
            selectionVector[i] = i;
        }
        selVecSize = VECEX_BYTE;
        return;
    }

    for (int i = 0; i < VECEX_BYTE; i += 64) {
        // todo: design a predicate tree for DNF

        // initialize mask
        SIMD_512iTYPE maskAND = _mm512_load_si512(&predMasks[preds[0][0]][i]);
        for (int oi = 1; oi < predORsize[0]; oi++) {
            SIMD_512iTYPE mor = _mm512_load_si512(&predMasks[preds[0][oi]][i]);
            maskAND = _mm512_or_si512(maskAND, mor);
        }

        for (int ai = 1; ai < predANDsize; ai++) {
            SIMD_512iTYPE maskOR =
                _mm512_load_si512(&predMasks[preds[ai][0]][i]);
            for (int oi = 1; oi < predORsize[ai]; oi++) {
                SIMD_512iTYPE mand =
                    _mm512_load_si512(&predMasks[preds[ai][oi]][i]);
                maskOR = _mm512_or_si512(maskOR, mand);
            }
            maskAND = _mm512_and_si512(maskAND, maskOR);
        }

        _mm512_store_si512(&mask[i >> 1], maskAND);
    }

    for (int i = 0; i < VECEX_BYTE16; i++) {
        SIMD_512iTYPE tids = _mm512_load_si512(&tupleIds[i << 4]);
        _mm512_mask_compressstoreu_epi32(&selectionVector[selVecSize], mask[i],
                                         tids);
        selVecSize += _popcnt32(mask[i]);
    }
}

inline void Executor::vselection() {
    vevalPreds();
    veval();
}

void Executor::projection() {
    DATA_TYPE buf[VECEX_BYTE];

    for (int si = 0; si < selVecSize; si++) {
        printf("| ");
        for (int k = 0; k < numProjKeys; k++) {
            if (keyTypes[k] == DOUBLE) {
                printf("%lf | ", bufArray[k][selectionVector[si]].d);
            } else if (keyTypes[k] == INT) {
                printf("%ld | ", bufArray[k][selectionVector[si]].i);
            } else if (keyTypes[k] == TEXT) {
                buf[sizes[k][si]] = (DATA_TYPE)0;
                memcpy(buf, bufArray[k][selectionVector[si]].p,
                       sizes[k][selectionVector[si]]);
                printf("%s | ", buf);
            }
        }
        printf("\n");
    }
}

void Executor::aggregation0() {
    for (int vk = 0; vk < aggVSize; vk++) {
        int vkk = aggContext->valueKeys[vk].key;
        AggFuncType ftype = aggContext->valueKeys[vk].ftype;

        if (aggContext->func == SUM) {
            for (int si = 0; si < selVecSize; si++) {
                int sv = selectionVector[si];
                agg[vk].d += bufArray[vkk][sv].d;
            }
        } else if (aggContext->func == COUNT) {
            aggCount[vk] += selVecSize;
        } else if (aggContext->func == AVG) {
            for (int si = 0; si < selVecSize; si++) {
                int sv = selectionVector[si];
                agg[vk].d += bufArray[vkk][sv].d;
            }
            aggCount[vk] += selVecSize;
        } else if (aggContext->func == MAX) {
            for (int si = 0; si < selVecSize; si++) {
                int sv = selectionVector[si];
                agg[vk].d =
                    ((agg[vk].d > bufArray[vkk][sv].d) ? agg[vk].d
                                                       : bufArray[vkk][sv].d);
            }
        } else if (aggContext->func == MIN) {
            for (int si = 0; si < selVecSize; si++) {
                int sv = selectionVector[si];
                agg[vk].d =
                    ((agg[vk].d < bufArray[vkk][sv].d) ? agg[vk].d
                                                       : bufArray[vkk][sv].d);
            }
        }
    }
}

void Executor::aggregation1() {
    int kk = aggContext->keys[0];

    for (int vk = 0; vk < aggVSize; vk++) {
        int vkk = aggContext->valueKeys[vk].key;
        AggFuncType ftype = aggContext->valueKeys[vk].ftype;
        if (ftype == COUNT) {
            for (int si = 0; si < selVecSize; si++) {
                int sv = selectionVector[si];
                // todo: UTF-8
                std::string s(reinterpret_cast<char *>(bufArray[kk][sv].p),
                              sizes[kk][sv]);
                if (!aggCountMap->find(s)) {
                    aggCountMap->assign(s, aggContext);
                }
                aggCountMap->count(s, vk);
            }
        } else if (ftype == DISTINCT) {
            for (int si = 0; si < selVecSize; si++) {
                int sv = selectionVector[si];
                // todo: UTF-8
                std::string s(reinterpret_cast<char *>(bufArray[kk][sv].p),
                              sizes[kk][sv]);
                if (!aggMap->find(s)) {
                    aggMap->assign(s, aggContext);
                }
            }
        } else if (aggContext->valueKeys[vk].ktype == DOUBLE) {
            if (ftype == SUM) {
                for (int si = 0; si < selVecSize; si++) {
                    int sv = selectionVector[si];
                    // todo: UTF-8
                    std::string s(reinterpret_cast<char *>(bufArray[kk][sv].p),
                                  sizes[kk][sv]);
                    if (!aggMap->find(s)) {
                        aggMap->assign(s, aggContext);
                    }
                    aggMap->sumDouble(s, bufArray[vkk][sv].d, vk);
                }
            } else if (ftype == AVG) {
                for (int si = 0; si < selVecSize; si++) {
                    int sv = selectionVector[si];
                    // todo: UTF-8
                    std::string s(reinterpret_cast<char *>(bufArray[kk][sv].p),
                                  sizes[kk][sv]);
                    if (!aggMap->find(s)) {
                        aggMap->assign(s, aggContext);
                        aggCountMap->assign(s, aggContext);
                    }
                    aggMap->sumDouble(s, bufArray[vkk][sv].d, vk);
                    aggCountMap->count(s, vk);
                }
            } else if (ftype == MAX) {
                for (int si = 0; si < selVecSize; si++) {
                    int sv = selectionVector[si];
                    // todo: UTF-8
                    std::string s(reinterpret_cast<char *>(bufArray[kk][sv].p),
                                  sizes[kk][sv]);
                    if (!aggMap->find(s)) {
                        aggMap->assign(s, aggContext);
                    }
                    aggMap->maxDouble(s, bufArray[vkk][sv].d, vk);
                }
            } else if (ftype == MIN) {
                for (int si = 0; si < selVecSize; si++) {
                    int sv = selectionVector[si];
                    // todo: UTF-8
                    std::string s(reinterpret_cast<char *>(bufArray[kk][sv].p),
                                  sizes[kk][sv]);
                    if (!aggMap->find(s)) {
                        aggMap->assign(s, aggContext);
                    }
                    aggMap->minDouble(s, bufArray[vkk][sv].d, vk);
                }
            }
        } else if (aggContext->valueKeys[vk].ktype == INT) {
            if (ftype == SUM) {
                for (int si = 0; si < selVecSize; si++) {
                    int sv = selectionVector[si];
                    // todo: UTF-8
                    std::string s(reinterpret_cast<char *>(bufArray[kk][sv].p),
                                  sizes[kk][sv]);
                    if (!aggMap->find(s)) {
                        aggMap->assign(s, aggContext);
                    }
                    aggMap->sumInt(s, bufArray[vkk][sv].i, vk);
                }
            } else if (ftype == AVG) {
                for (int si = 0; si < selVecSize; si++) {
                    int sv = selectionVector[si];
                    // todo: UTF-8
                    std::string s(reinterpret_cast<char *>(bufArray[kk][sv].p),
                                  sizes[kk][sv]);
                    if (!aggMap->find(s)) {
                        aggMap->assign(s, aggContext);
                        aggCountMap->assign(s, aggContext);
                    }
                    aggMap->sumInt(s, bufArray[vkk][sv].i, vk);
                    aggCountMap->count(s, vk);
                }
            } else if (ftype == MAX) {
                for (int si = 0; si < selVecSize; si++) {
                    int sv = selectionVector[si];
                    // todo: UTF-8
                    std::string s(reinterpret_cast<char *>(bufArray[kk][sv].p),
                                  sizes[kk][sv]);
                    if (!aggMap->find(s)) {
                        aggMap->assign(s, aggContext);
                    }
                    aggMap->maxInt(s, bufArray[vkk][sv].d, vk);
                }
            } else if (ftype == MIN) {
                for (int si = 0; si < selVecSize; si++) {
                    int sv = selectionVector[si];
                    // todo: UTF-8
                    std::string s(reinterpret_cast<char *>(bufArray[kk][sv].p),
                                  sizes[kk][sv]);
                    if (!aggMap->find(s)) {
                        aggMap->assign(s, aggContext);
                    }
                    aggMap->minInt(s, bufArray[vkk][sv].d, vk);
                }
            }
        }
    }
}

void Executor::aggregation() {
    int aggSize = aggContext->keys.size();
    if (aggSize > 1) {
        // todo: multiple keys
    } else if (aggSize == 1) {
        aggregation1();
    } else {
        aggregation0();
    }
}

void Executor::queryVExec() {
    vmaterialize();
    vselection();
    if (numProjKeys > 0) {
        projection();
    } else {
        aggregation();
    }

    tid = 0;
    selVecSize = 0;
}

void Executor::printAggregation0() {
    for (int vk = 0; vk < aggVSize; vk++) {
        AggFuncType ftype = aggContext->valueKeys[vk].ftype;
        if (ftype == COUNT) {
            printf("| %d |\n", aggCount[vk]);
        } else if (aggContext->valueKeys[vk].ktype == DOUBLE) {
            if (ftype == AVG) {
                printf("| %lf |\n", agg[vk].d / aggCount[vk]);
            } else {
                printf("| %lf |\n", agg[vk].d);
            }
        } else if (aggContext->valueKeys[vk].ktype == INT) {
            if (ftype == AVG) {
                printf("| %lf |\n", (double)agg[vk].i / aggCount[vk]);
            } else {
                printf("| %ld |\n", agg[vk].i);
            }
        }
    }
}

void Executor::printAggregation1() {
    AggregationValueMap::HTType &ht = aggMap->getHashTable();
    AggregationCountMap::HTType &cht = aggCountMap->getHashTable();
    auto begin = ht.begin();
    auto end = ht.begin();
    if (limit) {
        for (int i = 0; i < limit; i++) {
            ++end;
        }
    } else {
        end = ht.end();
    }

    for (auto it = begin; it != end; ++it) {
        printf("| %s |", it->first.c_str());
        for (int vk = 0; vk < aggVSize; vk++) {
            AggFuncType ftype = aggContext->valueKeys[vk].ftype;
            if (aggContext->valueKeys[vk].ftype == COUNT) {
                printf(" %d |", cht.at(it->first)[vk]);
            } else if (aggContext->valueKeys[vk].ftype == DISTINCT) {
                break;
            } else if (aggContext->valueKeys[vk].ktype == DOUBLE) {
                if (ftype == AVG) {
                    printf(" %lf |",
                           ht.at(it->first)[vk].d / cht.at(it->first)[vk]);
                } else {
                    printf(" %lf |", ht.at(it->first)[vk].d);
                }
            } else if (aggContext->valueKeys[vk].ktype == INT) {
                if (ftype == AVG) {
                    printf(" %lf |",
                           ht.at(it->first)[vk].i / cht.at(it->first)[vk]);
                } else {
                    printf(" %ld |", ht.at(it->first)[vk].i);
                }
            }
        }
        printf("\n");
    }
}

void Executor::queryEndExec() {
    if (numProjKeys > 0) {
    } else {
        if (aggContext->keys.size() > 1) {
        } else if (aggContext->keys.size() == 1) {
            printAggregation1();
        } else {
            printAggregation0();
        }
    }
}

void Executor::exec(DATA_TYPE *_data, SIZE_TYPE _size) {
    data = _data, size = _size;

#if (defined BENCH)
    double ex_time = 0.0;
    timeval lex1, lex2;
    gettimeofday(&lex1, NULL);
#endif

    while (i < size) {
        switch (kindTable[ctx.currentState]) {
            case ORDERED:
                cmpestri_ord(ctx.currentState);
                break;
            case ANY:
                cmpestri_any(ctx.currentState);
                break;
            case RANGES:
                cmpestri_ranges(ctx.currentState);
                break;
            case C:
                if (ctx.currentState == INIT_STATE) {
                    resetContext();
                }

                if (charStartTable[ctx.currentState] > 0) {
                    startSubMatch(charStartTable[ctx.currentState]);
                }
                ctx.currentState = charTable[ctx.currentState][(int)data[i++]];
                if (charEndTable[ctx.currentState] > 0) {
                    endSubMatch(charEndTable[ctx.currentState]);
                }
                break;
            default:
                if (ctx.recentAcceptState) {
                    // todo: change it according to query plan
                    if (end->id > 0) {
                        while (!startStack.empty()) {
                            if (startStack.top().id == end->id) {
                                subMatches[end->id - 1].start =
                                    startStack.top().index;
                                subMatches[end->id - 1].end = end->index;
                            }
                            startStack.pop();
                        }
                        end->id = 0;
                    }
                    for (int s = 0; s < subMatchSize; s++) {
                        indexes[s][tid] = subMatches[s].start;
                        sizes[s][tid] = subMatches[s].end - subMatches[s].start;
                    }
                    tid++;
                    if (tid == VECEX_BYTE) {
                        queryVExec();
                    }

                    i = ctx.recentAcceptIndex + 1;
                }
                ctx.currentState = INIT_STATE;
                break;
        }

        if (acceptStates.find(ctx.currentState) != acceptStates.end()) {
            ctx.recentAcceptState = ctx.currentState;
            ctx.recentAcceptIndex = i;
        }
    }

    // queryExec(tid);
    // todo: add non-vectorized execution
    queryEndExec();

#if (defined BENCH)
    gettimeofday(&lex2, NULL);
    ex_time =
        (lex2.tv_sec - lex1.tv_sec) + (lex2.tv_usec - lex1.tv_usec) * 0.000001;
    printf("#BENCH_exec: %lf s\n\n", ex_time);
#endif
}
