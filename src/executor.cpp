#include "executor/executor.hpp"

#include "general/clock.hpp"

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
    endPredIdTable = new int[stateSize]();
    std::vector<DFA::SubMatchStates> &subMatchStates = vfa->getSubMatches();
    subMatchSize = subMatchStates.size();
    keyTypes = new Type[subMatchSize];
    for (DFA::SubMatchStates &sms : subMatchStates) {
        keyTypes[sms.id] = sms.type;
        // sms.predID
        endPredIdTable[sms.endState] = sms.predID;
        if (sms.isAnyStart) {
            anyStartTable[sms.startState] = sms.id + 1;
        } else {
            charStartTable[sms.startState] = sms.id + 1;
        }
        if (sms.isAnyEnd) {
            anyEndTable[sms.endState] = sms.id + 1;
        } else {
            charEndTable[sms.endState] = sms.id + 1;
        }
    }

    end = new SubMatchNode;
    if (subMatchSize > 0) {
        subMatches = new SubMatch[subMatchSize];
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

// void Executor::setVQuery(QueryContext *query) {
// indexes = new SIZE_TYPE *[subMatchSize];
// sizes = new SIZE_TYPE *[subMatchSize];
// for (int k = 0; k < subMatchSize; k++) {
//     indexes[k] = new (std::align_val_t{VECEX_SIZE});
//     sizes[k] = new (std::align_val_t{VECEX_SIZE}) SIZE_TYPE[VECEX_BYTE];
// }

// bufArray = new data64 *[subMatchSize];
// for (int k = 0; k < subMatchSize; k++) {
//     bufArray[k] = new (std::align_val_t{VECEX_SIZE}) data64[VECEX_BYTE];
// }

// predMasks = new uint8_t *[numPreds];
// for (int k = 0; k < numPreds; k++) {
//     predMasks[k] = new (std::align_val_t{VECEX_SIZE}) uint8_t[VECEX_BYTE4];
// }
// mask = new (std::align_val_t{VECEX_SIZE}) uint16_t[VECEX_BYTE16];

// tupleIds = new (std::align_val_t{VECEX_SIZE}) int[VECEX_BYTE];
// for (int i = 0; i < VECEX_BYTE; i++) {
//     tupleIds[i] = i;
// }
// selectionVector = new int[VECEX_BYTE];

// QueryContext::Selection &sel = query->getSelection();
// isCNF = sel.isCNF;
// numPreds = sel.numPreds;
// if (isCNF) {
//     predANDsize = sel.preds.size();
//     predORsize = new int[predANDsize];
//     for (int ai = 0; ai < predANDsize; ai++) {
//         predORsize[ai] = sel.preds[ai].size();
//     }
//     preds = new int *[predANDsize];
//     for (int ai = 0; ai < predANDsize; ai++) {
//         preds[ai] = new int[predORsize[ai]];
//         for (int oi = 0; oi < predORsize[ai]; oi++) {
//             preds[ai][oi] = sel.preds[ai][oi];
//         }
//     }
// } else {
//     // todo: add to DNF
// }

// predTypes = new Type[numPreds];
// predTrees = new QueryContext::OpTree[numPreds];
// for (int k = 0; k < numPreds; k++) {
//     predTypes[k] = sel.predTypes[k];
//     predTrees[k] = sel.predTrees[k];
// }
// }

void Executor::setQuery(QueryContext *query) {
    tuple = new data64[subMatchSize];
    tsize = new SIZE_TYPE[subMatchSize];

    StatementList *stmts = query->getStatements();
    for (StatementList *s = stmts; s != NULL; s = s->next) {
        stmtList.push_front(*s->stmt);
        if (s->stmt->httype != NONE_HT) {
            // extract aggregate function
            OpTree *op;
            std::queue<OpTree *> bfsQueue;
            bfsQueue.push(s->stmt->expr);
            while (!bfsQueue.empty()) {
                op = bfsQueue.front();
                if (op->evalType == OP) {
                    bfsQueue.push(op->left);
                    bfsQueue.push(op->right);
                } else if (op->evalType == AGGFUNC) {
                    Aggregation agc;
                    agc.type = op->type;
                    agc.ftype = op->ftype;
                    agc.keyId = op->varKeyId;
                    aggContext.push_back(agc);
                    op->aggId = aggContext.size() - 1;
                }
                bfsQueue.pop();
            }
        }
        httype = intersectionHTType(s->stmt->httype, httype);
    }

    ptree = query->getPredTree();
    textPredNum = query->getTextPredNum();
    textPredResults = new int[textPredNum + 1];

    gKeyVec = query->getGKeys();
    int aggVSize = aggContext.size();
    if (gKeyVec.size() == 1) {
        aggMap = new AggregationValueMap(aggVSize);
        aggCountMap = new AggregationCountMap(aggVSize);
    } else if (gKeyVec.size() == 0) {
        agg = new data64[aggVSize];
        aggCount = new int[aggVSize];
        for (int vk = 0; vk < aggVSize; vk++) {
            AggFuncType ftype = aggContext[vk].ftype;
            if (aggContext[vk].type == DOUBLE) {
                if (ftype == MIN) {
                    agg[vk].d = DBL_MAX;
                } else if (ftype == MAX) {
                    agg[vk].d = -DBL_MAX;
                } else {
                    agg[vk].d = 0.0;
                }
            } else if (aggContext[vk].type == INT) {
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
    }  // todo: multiple keys

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
        if (endPredIdTable[cur_state] > 0) {
            textPredResults[endPredIdTable[cur_state]] = 1;
        }
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
        if (endPredIdTable[cur_state] > 0) {
            textPredResults[endPredIdTable[cur_state]] = 1;
        }
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
        if (endPredIdTable[cur_state] > 0) {
            textPredResults[endPredIdTable[cur_state]] = 1;
        }
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
        if (endPredIdTable[cur_state] > 0) {
            textPredResults[endPredIdTable[cur_state]] = 1;
        }
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
    for (int ti = 0; ti < textPredNum + 1; ti++) {
        textPredResults[ti] = 0;
    }
    while (!startStack.empty()) {
        startStack.pop();
    }
}

#if (defined VECEXEC)

void Executor::materialize(int tid) {
    DATA_TYPE buf[128];
    for (int s = 0; s < subMatchSize; s++) {
        uint32_t start = subMatches[s].start;
        uint32_t size = subMatches[s].end - subMatches[s].start;
        switch (keyTypes[s]) {
            case DOUBLE:
                memcpy(buf, &data[start], size);
                buf[size] = (DATA_TYPE)0;
                bufArray[s][tid].d = atof((char *)buf);
                // bufArray[s][tid].d = fatod(&data[start]);
                break;
            case INT:
                memcpy(buf, &data[start], size);
                buf[size] = (DATA_TYPE)0;
                bufArray[s][tid].i = (int64_t)atoi((char *)buf);
                // bufArray[s][tid].i = fatoi(&data[start], size);
                break;
            case TEXT:
                bufArray[s][tid].p = data + start;
                sizes[s][tid] = size;
                break;
            default:
                break;
        }
    }
}

void Executor::vmaterialize() {
    for (int s = 0; s < subMatchSize; s++) {
        SIMD_512iTYPE base = _mm512_set1_epi64(reinterpret_cast<int64_t>(data));
        DATA_TYPE buf[128];
        switch (keyTypes[s]) {
            case DOUBLE:
#if (defined VUNROLL)
#pragma unroll(4)
#endif
                for (int ti = 0; ti < VECEX_BYTE; ti++) {
                    memcpy(buf, &data[indexes[s][ti]], sizes[s][ti]);
                    buf[sizes[s][ti]] = (DATA_TYPE)0;
                    double d = atof((char *)buf);
                    bufArray[s][ti].d = d;
                }
                break;
            case INT:
#if (defined VUNROLL)
#pragma unroll(4)
#endif
                for (int ti = 0; ti < VECEX_BYTE; ti++) {
                    memcpy(buf, &data[indexes[s][ti]], sizes[s][ti]);
                    buf[sizes[s][ti]] = (DATA_TYPE)0;
                    int64_t n = (int64_t)atoi((char *)buf);
                    bufArray[s][ti].i = n;
                }
                break;
            case TEXT:
#if (defined VUNROLL)
#pragma unroll(4)
#endif
                for (int ti = 0; ti < VECEX_BYTE; ti += 8) {
                    SIMD_256iTYPE offsets = _mm256_load_si256(
                        reinterpret_cast<SIMD_256iTYPE *>(&indexes[s][ti]));
                    SIMD_512iTYPE adrs =
                        _mm512_add_epi64(base, _mm512_cvtepi32_epi64(offsets));
                    _mm512_store_si512(
                        reinterpret_cast<SIMD_512iTYPE *>(&bufArray[s][ti]),
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

#if (defined VUNROLL)
#pragma unroll(4)
#endif
    if (isCNF) {
        for (int i = 0; i < VECEX_BYTE; i += 64) {
            // todo: design a predicate tree for DNF

            // initialize mask
            SIMD_512iTYPE maskAND =
                _mm512_load_si512(&predMasks[preds[0][0]][i]);
            for (int oi = 1; oi < predORsize[0]; oi++) {
                SIMD_512iTYPE mor =
                    _mm512_load_si512(&predMasks[preds[0][oi]][i]);
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
    } else {
        for (int i = 0; i < VECEX_BYTE; i += 64) {
            // todo: design a predicate tree for DNF

            // initialize mask
            SIMD_512iTYPE maskOR =
                _mm512_load_si512(&predMasks[preds[0][0]][i]);
            for (int oi = 1; oi < predORsize[0]; oi++) {
                SIMD_512iTYPE mand =
                    _mm512_load_si512(&predMasks[preds[0][oi]][i]);
                maskOR = _mm512_and_si512(maskOR, mand);
            }

            for (int ai = 1; ai < predANDsize; ai++) {
                SIMD_512iTYPE maskAND =
                    _mm512_load_si512(&predMasks[preds[ai][0]][i]);
                for (int oi = 1; oi < predORsize[ai]; oi++) {
                    SIMD_512iTYPE mor =
                        _mm512_load_si512(&predMasks[preds[ai][oi]][i]);
                    maskAND = _mm512_and_si512(maskAND, mor);
                }
                maskOR = _mm512_or_si512(maskOR, maskAND);
            }

            _mm512_store_si512(&mask[i >> 1], maskOR);
        }
    }

#if (defined VUNROLL)
#pragma unroll(4)
#endif
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

void Executor::vprojection() {
    DATA_TYPE buf[VECEX_BYTE];

    for (int si = 0; si < selVecSize; si++) {
        printf("| ");
        for (int k = 0; k < numProjs; k++) {
            if (keyTypes[k] == DOUBLE) {
                printf("%lf | ", bufArray[k][selectionVector[si]].d);
            } else if (keyTypes[k] == INT) {
                printf("%ld | ", bufArray[k][selectionVector[si]].i);
            } else if (keyTypes[k] == TEXT) {
                buf[sizes[k][selectionVector[si]]] = (DATA_TYPE)0;
                memcpy(buf, bufArray[k][selectionVector[si]].p,
                       sizes[k][selectionVector[si]]);
                printf("%s | ", buf);
            }
        }
        printf("\n");
    }
}

void Executor::vaggregation0() {
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

void Executor::vaggregation1() {
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
                    aggMap->maxInt(s, bufArray[vkk][sv].i, vk);
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
                    aggMap->minInt(s, bufArray[vkk][sv].i, vk);
                }
            }
        }
    }
}

inline void Executor::vaggregation() {
    int aggSize = aggContext->keys.size();
    if (aggSize > 1) {
        // todo: multiple keys
    } else if (aggSize == 1) {
        vaggregation1();
    } else {
        vaggregation0();
    }
}

inline void Executor::queryVExec() {
    if (tid != VECEX_BYTE) return;

    vselection();
    if (numProjs > 0) {
        vprojection();
    } else {
        vaggregation();
    }

    tid = 0;
    selVecSize = 0;
}

#else

void Executor::printColumnNames() {
    int id = 1;

    printf("|");
    for (auto stmt = stmtList.begin(); stmt != stmtList.end(); ++stmt) {
        if (stmt->name == NULL) {
            printf(" %d |", id);
        } else {
            printf(" %s |", stmt->name);
        }
        id++;
    }
    printf("\n");
}

void Executor::queryStartExec() { printColumnNames(); }

void Executor::materialize() {
    DATA_TYPE buf[128];
    for (int s = 0; s < subMatchSize; s++) {
        uint32_t start = subMatches[s].start;
        uint32_t size = subMatches[s].end - subMatches[s].start;
        switch (keyTypes[s]) {
            case DOUBLE:
                memcpy(buf, &data[start], size);
                buf[size] = (DATA_TYPE)0;
                tuple[s].d = atof((char *)buf);
                // tuple[s].d = fatod(&data[start]);
                break;
            case INT:
                memcpy(buf, &data[start], size);
                buf[size] = (DATA_TYPE)0;
                tuple[s].i = (int64_t)atoi((char *)buf);
                // tuple[s].i = fatoi(&data[start], size);
                break;
            case TEXT:
                tuple[s].p = data + start;
                tsize[s] = size;
                break;
            default:
                break;
        }
    }
}

template <typename Value>
Value Executor::evalFunc1Op(OpTree *tree, data64 *vhtv, int *chtv) {
    if (tree->evalType == OP) {
        if (tree->opType == ADD) {
            return evalFunc1Op<Value>(tree->left, vhtv, chtv) +
                   evalFunc1Op<Value>(tree->right, vhtv, chtv);
        } else if (tree->opType == SUB) {
            return evalFunc1Op<Value>(tree->left, vhtv, chtv) -
                   evalFunc1Op<Value>(tree->right, vhtv, chtv);
        } else if (tree->opType == MUL) {
            return evalFunc1Op<Value>(tree->left, vhtv, chtv) *
                   evalFunc1Op<Value>(tree->right, vhtv, chtv);
        } else if (tree->opType == DIV) {
            return evalFunc1Op<Value>(tree->left, vhtv, chtv) /
                   evalFunc1Op<Value>(tree->right, vhtv, chtv);
        }
    } else if (tree->evalType == AGGFUNC) {
        if (tree->ftype == COUNT) {
            return static_cast<Value>(chtv[tree->aggId]);
        } else if (tree->type == DOUBLE) {
            if (tree->ftype == AVG) {
                return static_cast<Value>(vhtv[tree->aggId].d /
                                          chtv[tree->aggId]);
            } else {
                return static_cast<Value>(vhtv[tree->aggId].d);
            }
        } else if (tree->type == INT) {
            if (tree->ftype == AVG) {
                return static_cast<Value>(vhtv[tree->aggId].i /
                                          chtv[tree->aggId]);
            } else {
                return static_cast<Value>(vhtv[tree->aggId].i);
            }
        }
    } else {
        return evalOp<Value>(tree);
    }
}

template <typename Value>
Value Executor::evalOp(OpTree *tree) {
    if (tree->evalType == OP) {
        if (tree->opType == ADD) {
            return evalOp<Value>(tree->left) + evalOp<Value>(tree->right);
        } else if (tree->opType == SUB) {
            return evalOp<Value>(tree->left) - evalOp<Value>(tree->right);
        } else if (tree->opType == MUL) {
            return evalOp<Value>(tree->left) * evalOp<Value>(tree->right);
        } else if (tree->opType == DIV) {
            return evalOp<Value>(tree->left) / evalOp<Value>(tree->right);
        }
    } else if (tree->evalType == VAR) {
        if (tree->type == DOUBLE) {
            return static_cast<Value>(tuple[tree->varKeyId].d);
        } else if (tree->type == INT) {
            return static_cast<Value>(tuple[tree->varKeyId].i);
        }
    } else if (tree->evalType == AGGFUNC) {
        // in keynum == 0 case
        if (tree->ftype == COUNT) {
            return static_cast<Value>(aggCount[tree->aggId]);
        } else if (tree->ftype == AVG) {
            if (tree->type == DOUBLE) {
                return static_cast<Value>(
                    agg[tree->aggId].d /
                    static_cast<double>(aggCount[tree->aggId]));
            } else if (tree->type == INT) {
                return static_cast<Value>(
                    static_cast<double>(agg[tree->aggId].i) /
                    static_cast<double>(aggCount[tree->aggId]));
            }
        } else {
            if (tree->type == DOUBLE) {
                return static_cast<Value>(agg[tree->aggId].d);
            } else if (tree->type == INT) {
                return static_cast<Value>(agg[tree->aggId].i);
            }
        }
    } else {
        if (tree->type == DOUBLE) {
            return static_cast<Value>(tree->constData.d);
        } else if (tree->type == INT) {
            return static_cast<Value>(tree->constData.i);
        }
    }
}

bool Executor::evalPred(OpTree *tree) {
    if (tree->type == DOUBLE) {
        double lvalue = tuple[tree->left->varKeyId].d;
        double rvalue = evalOp<double>(tree->right);
        if (tree->opType == EQUAL && lvalue == rvalue)
            return true;
        else if (tree->opType == NEQUAL && lvalue != rvalue)
            return true;
        else if (tree->opType == LESS && lvalue < rvalue)
            return true;
        else if (tree->opType == LESSEQ && lvalue <= rvalue)
            return true;
        else if (tree->opType == GREATEQ && lvalue >= rvalue)
            return true;
        else if (tree->opType == GREATER && lvalue > rvalue)
            return true;
        else
            return false;
    } else if (tree->type == INT) {
        int64_t lvalue = tuple[tree->left->varKeyId].i;
        int64_t rvalue = evalOp<int64_t>(tree->right);
        if (tree->opType == EQUAL && lvalue == rvalue)
            return true;
        else if (tree->opType == NEQUAL && lvalue != rvalue)
            return true;
        else if (tree->opType == LESS && lvalue < rvalue)
            return true;
        else if (tree->opType == LESSEQ && lvalue <= rvalue)
            return true;
        else if (tree->opType == GREATEQ && lvalue >= rvalue)
            return true;
        else if (tree->opType == GREATER && lvalue > rvalue)
            return true;
        else
            return false;
    } else if (tree->type == TEXT) {
        if (tree->right->evalType == VAR) {
            if (tree->opType == EQUAL) {
                SIMD_512iTYPE l =
                    _mm512_loadu_epi8(tuple[tree->left->varKeyId].p);
                SIMD_512iTYPE r =
                    _mm512_loadu_epi8(tuple[tree->right->varKeyId].p);
                uint64_t m = _mm512_cmpeq_epi8_mask(l, r);
                if (__builtin_ffsll(~m | ((uint64_t)1 << 63)) >
                    tsize[tree->left->varKeyId]) {
                    return true;
                } else {
                    return false;
                }
            } else if (tree->opType == NEQUAL) {
                SIMD_512iTYPE l =
                    _mm512_loadu_epi8(tuple[tree->left->varKeyId].p);
                SIMD_512iTYPE r =
                    _mm512_loadu_epi8(tuple[tree->right->varKeyId].p);
                uint64_t m = _mm512_cmpeq_epi8_mask(l, r);
                if (__builtin_ffsll(m | ((uint64_t)1 << 63)) >
                    tsize[tree->left->varKeyId]) {
                    return true;
                } else {
                    return false;
                }
            } else {
                return false;
            }
        } else if (tree->right->evalType == CONST) {
            return textPredResults[tree->textPredId] > 0 ? true : false;
            // if (tree->opType == EQUAL) {
            // SIMD_512iTYPE r =
            //     _mm512_loadu_epi8(tree->right->constData.p + 1);
            // SIMD_512iTYPE l =
            //     _mm512_loadu_epi8(tuple[tree->left->varKeyId].p);
            // uint64_t m = _mm512_cmpeq_epi8_mask(l, r);
            // if (__builtin_ffsll(~m | ((uint64_t)1 << 63)) >
            //     tsize[tree->left->varKeyId]) {
            //     return true;
            // } else {
            //     return false;
            // }
            // } else if (tree->opType == NEQUAL) {
            // SIMD_512iTYPE r =
            //     _mm512_loadu_epi8(tree->right->constData.p + 1);
            // SIMD_512iTYPE l =
            //     _mm512_loadu_epi8(tuple[tree->left->varKeyId].p);
            // uint64_t m = _mm512_cmpeq_epi8_mask(l, r);
            // if (__builtin_ffsll(m | ((uint64_t)1 << 63)) >
            //     tsize[tree->left->varKeyId]) {
            //     return true;
            // } else {
            //     return false;
            // }
            // } else {
            // if (re2::RE2::FullMatch(
            //         re2::StringPiece((char
            //         *)tuple[tree->left->varKeyId].p,
            //                          32),
            //         pt1, &s, &t)) {
            //     return true;
            // } else {
            //     return false;
            // }
            // if (std::regex_search(
            //         std::string((char *)tuple[tree->left->varKey].p, 32),
            //         pattern[tree->left->varKey])) {
            //     return true;
            // } else {
            //     return false;
            // }
            // }
        }
    }
    return false;
}

bool Executor::evalCond(PredTree *ptree) {
    bool l, r;
    if (ptree->evalType == COND) {
        l = evalCond(ptree->left);
        r = evalCond(ptree->right);
        if (ptree->ctype == AND) {
            return l && r;
        } else if (ptree->ctype == OR) {
            return l || r;
        }
    } else if (ptree->evalType == PRED) {
        return evalPred(ptree->pred);
    } else {
        return false;
    }
}

bool Executor::selection() {
    if (ptree != NULL) {
        return evalCond(ptree);
    } else {
        return true;
    }
}

void Executor::projection() {
    printf("| ");
    for (auto stmt = stmtList.begin(); stmt != stmtList.end(); ++stmt) {
        if (stmt->expr->type == DOUBLE) {
            double v = evalOp<double>(stmt->expr);
            printf("%lf | ", v);
        } else if (stmt->expr->type == INT) {
            int64_t v = evalOp<int64_t>(stmt->expr);
            printf("%ld | ", v);
        } else if (stmt->expr->type == TEXT) {
            DATA_TYPE buf[128];
            int k = stmt->expr->varKeyId;
            buf[tsize[k]] = (DATA_TYPE)0;
            memcpy(buf, tuple[k].p, tsize[k]);
            printf("%s | ", buf);
        }
    }
    printf("\n");
}

void Executor::aggregation0() {
    for (int vk = 0; vk < aggContext.size(); vk++) {
        int vkk = aggContext[vk].keyId;
        switch (aggContext[vk].ftype) {
            case SUM:
                agg[vk].d += tuple[vkk].d;
                break;
            case COUNT:
                aggCount[vk]++;
                break;
            case AVG:
                agg[vk].d += tuple[vkk].d;
                aggCount[vk]++;
                break;
            case MAX:
                agg[vk].d =
                    ((agg[vk].d > tuple[vkk].d) ? agg[vk].d : tuple[vkk].d);
                break;
            case MIN:
                agg[vk].d =
                    ((agg[vk].d < tuple[vkk].d) ? agg[vk].d : tuple[vkk].d);
                break;
            default:
                break;
        }
    }
}

void Executor::aggregation1() {
    int kk = gKeyVec[0].id;

    for (int vk = 0; vk < aggContext.size(); vk++) {
        int vkk = aggContext[vk].keyId;
        AggFuncType ftype = aggContext[vk].ftype;
        if (ftype == COUNT) {
            // todo: UTF-8
            std::string s(reinterpret_cast<char *>(tuple[kk].p), tsize[kk]);
            if (!aggCountMap->find(s)) {
                aggCountMap->assign(s);
            }
            aggCountMap->count(s, vk);
        } else if (ftype == DISTINCT) {
            std::string s(reinterpret_cast<char *>(tuple[kk].p), tsize[kk]);
            if (!aggCountMap->find(s)) {
                aggCountMap->assign(s);
            }
        } else if (aggContext[vk].type == DOUBLE) {
            if (ftype == SUM) {
                std::string s(reinterpret_cast<char *>(tuple[kk].p), tsize[kk]);
                if (!aggMap->find(s)) {
                    aggMap->assign(s, aggContext);
                }
                aggMap->sumDouble(s, tuple[vkk].d, vk);
            } else if (ftype == AVG) {
                std::string s(reinterpret_cast<char *>(tuple[kk].p), tsize[kk]);
                if (!aggMap->find(s)) {
                    aggMap->assign(s, aggContext);
                    aggCountMap->assign(s);
                }
                aggMap->sumDouble(s, tuple[vkk].d, vk);
                aggCountMap->count(s, vk);
            } else if (ftype == MAX) {
                std::string s(reinterpret_cast<char *>(tuple[kk].p), tsize[kk]);
                if (!aggMap->find(s)) {
                    aggMap->assign(s, aggContext);
                }
                aggMap->maxDouble(s, tuple[vkk].d, vk);
            } else if (ftype == MIN) {
                std::string s(reinterpret_cast<char *>(tuple[kk].p), tsize[kk]);
                if (!aggMap->find(s)) {
                    aggMap->assign(s, aggContext);
                }
                aggMap->minDouble(s, tuple[vkk].d, vk);
            }
        } else if (aggContext[vk].type == INT) {
            if (ftype == SUM) {
                std::string s(reinterpret_cast<char *>(tuple[kk].p), tsize[kk]);
                if (!aggMap->find(s)) {
                    aggMap->assign(s, aggContext);
                }
                aggMap->sumInt(s, tuple[vkk].i, vk);
            } else if (ftype == AVG) {
                std::string s(reinterpret_cast<char *>(tuple[kk].p), tsize[kk]);
                if (!aggMap->find(s)) {
                    aggMap->assign(s, aggContext);
                    aggCountMap->assign(s);
                }
                aggMap->sumInt(s, tuple[vkk].i, vk);
                aggCountMap->count(s, vk);
            } else if (ftype == MAX) {
                std::string s(reinterpret_cast<char *>(tuple[kk].p), tsize[kk]);
                if (!aggMap->find(s)) {
                    aggMap->assign(s, aggContext);
                }
                aggMap->maxInt(s, tuple[vkk].i, vk);
            } else if (ftype == MIN) {
                std::string s(reinterpret_cast<char *>(tuple[kk].p), tsize[kk]);
                if (!aggMap->find(s)) {
                    aggMap->assign(s, aggContext);
                }
                aggMap->minInt(s, tuple[vkk].i, vk);
            }
        }
    }
}

void Executor::aggregation() {
    int aggSize = gKeyVec.size();
    if (aggSize > 1) {
        // todo: multiple keys
    } else if (aggSize == 1) {
        aggregation1();
    } else {
        aggregation0();
    }
}

void Executor::queryExec() {
    materialize();

    if (selection()) {
        if (httype == NONE_HT) {
            projection();
        } else {
            aggregation();
        }
    }
}
#endif

void Executor::printAggregation0() {
    printf("|");
    for (auto stmt = stmtList.begin(); stmt != stmtList.end(); ++stmt) {
        if (stmt->expr->type == INT) {
            printf(" %ld |", evalOp<int64_t>(stmt->expr));
        } else if (stmt->expr->type == DOUBLE) {
            printf(" %lf |", evalOp<double>(stmt->expr));
        } else {
        }
    }
    printf("\n");
}

void Executor::printAggregation1() {
    AggregationValueMap::HTType &vht = aggMap->getHashTable();
    AggregationCountMap::HTType &cht = aggCountMap->getHashTable();
    auto vbegin = vht.begin();
    auto vend = vht.begin();
    auto cbegin = cht.begin();
    auto cend = cht.begin();

    if (httype == COUNT_HT) {
        if (limit && limit <= cht.size()) {
            for (int i = 0; i < limit; i++) {
                ++cend;
            }
        } else {
            cend = cht.end();
        }
    } else if (httype == VALUE_HT || httype == BOTH_HT) {
        if (limit && limit <= vht.size()) {
            for (int i = 0; i < limit; i++) {
                ++vend;
            }
        } else {
            vend = vht.end();
        }
    }

    if (httype == COUNT_HT) {
        for (auto it = cbegin; it != cend; ++it) {
            printf("|");
            for (auto stmt = stmtList.begin(); stmt != stmtList.end(); ++stmt) {
                if (stmt->httype == NONE_HT) {
                    printf(" %s |", it->first.c_str());
                } else {
                    if (stmt->expr->type == INT) {
                        printf(" %ld |",
                               evalFunc1Op<int64_t>(stmt->expr, NULL,
                                                    cht.at(it->first)));
                    } else if (stmt->expr->type == DOUBLE) {
                        printf(" %lf |",
                               evalFunc1Op<double>(stmt->expr, NULL,
                                                   cht.at(it->first)));
                    }
                }
            }
            printf("\n");
        }
    } else if (httype == VALUE_HT || httype == BOTH_HT) {
        for (auto it = vbegin; it != vend; ++it) {
            printf("|");
            for (auto stmt = stmtList.begin(); stmt != stmtList.end(); ++stmt) {
                if (stmt->httype == NONE_HT) {
                    printf(" %s |", it->first.c_str());
                } else {
                    if (stmt->expr->type == INT) {
                        int64_t avi = 0;
                        if (stmt->httype == COUNT_HT) {
                            avi = evalFunc1Op<int64_t>(stmt->expr, NULL,
                                                       cht.at(it->first));
                        } else if (stmt->httype == VALUE_HT) {
                            avi = evalFunc1Op<int64_t>(stmt->expr,
                                                       vht.at(it->first), NULL);
                        } else if (stmt->httype == BOTH_HT) {
                            avi = evalFunc1Op<int64_t>(stmt->expr,
                                                       vht.at(it->first),
                                                       cht.at(it->first));
                        }
                        printf(" %ld |", avi);
                    } else if (stmt->expr->type == DOUBLE) {
                        double avd = 0.0;
                        if (stmt->httype == COUNT_HT) {
                            avd = evalFunc1Op<double>(stmt->expr, NULL,
                                                      cht.at(it->first));
                        } else if (stmt->httype == VALUE_HT) {
                            avd = evalFunc1Op<double>(stmt->expr,
                                                      vht.at(it->first), NULL);
                        } else if (stmt->httype == BOTH_HT) {
                            avd = evalFunc1Op<double>(stmt->expr,
                                                      vht.at(it->first),
                                                      cht.at(it->first));
                        }
                        printf(" %lf |", avd);
                    }
                }
            }
            printf("\n");
        }
    }
}

void Executor::queryEndExec() {
    if (httype == NONE_HT) {
    } else {
        if (gKeyVec.size() > 1) {
        } else if (gKeyVec.size() == 1) {
            printAggregation1();
        } else {
            printAggregation0();
        }
    }
}

void Executor::exec(DATA_TYPE *_data, SIZE_TYPE _size) {
    data = _data, size = _size;

#if (defined BENCH)
    // cpu_clock_counter_t t = mk_cpu_clock_counter();
    // long long c0 = cpu_clock_counter_get(t);

    double ex_time = 0.0;
    timeval lex1, lex2;
    gettimeofday(&lex1, NULL);
#endif
    queryStartExec();

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
                    if (endPredIdTable[ctx.currentState] > 0) {
                        textPredResults[endPredIdTable[ctx.currentState]] = 1;
                    }
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
#if (defined NOEXEC)
                    if (subMatches[0].start - subMatches[1].start == 0) {
                        tid++;
                    }
#elif (defined VECEXEC)
#if (defined BENCH_CLOCK)
                    uint64_t clock_start, clock_end;
                    clock_start = __rdtsc();
#endif
                    materialize(tid);
#if (defined BENCH_CLOCK)
                    clock_end = __rdtsc();
                    vmatclocks += clock_end - clock_start;
#endif
                    tid++;
                    queryVExec();
#else
                    queryExec();
#endif

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

#if (defined NOEXEC)
#else
    queryEndExec();
#endif

#if (defined BENCH)
    gettimeofday(&lex2, NULL);
    ex_time =
        (lex2.tv_sec - lex1.tv_sec) + (lex2.tv_usec - lex1.tv_usec) * 0.000001;
    printf("#BENCH_exec: %lf s\n\n", ex_time);

// long long c1 = cpu_clock_counter_get(t);
// printf("exec clock: %ld\n", c1 - c0);
#endif
}
