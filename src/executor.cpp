#include "executor.hpp"

using namespace vlex;

Executor::Executor() {}
Executor::Executor(VectFA *vfa, QueryContext *query, SIZE_TYPE _start) {
    setVFA(vfa, _start);
    setQuery(query);
}

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

    predMasks = new SIMD_256iTYPE *[numPreds];
    for (int k = 0; k < numPreds; k++) {
        predMasks[k] = new SIMD_256iTYPE[VECEX_BYTE4];
    }

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

    selectionVector = new int[VECEX_BYTE];
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
            prede.evaluateText(&predTrees[sk], data, indexes, sizes);
        }
    }
}

void Executor::veval() {
    int svi = 0;
    for (int i = 0; i < VECEX_BYTE4; i++) {
        // todo: design a predicate tree for DNF

        // initialize mask
        SIMD_256iTYPE maskAND = predMasks[preds[0][0]][i];
        for (int oi = 1; oi < predORsize[0]; oi++) {
            maskAND = _mm256_or_si256(maskAND, predMasks[preds[0][oi]][i]);
        }

        for (int ai = 1; ai < predANDsize; ai++) {
            SIMD_256iTYPE maskOR = predMasks[preds[ai][0]][i];
            for (int oi = 1; oi < predORsize[ai]; oi++) {
                maskOR = _mm256_or_si256(maskOR, predMasks[preds[ai][oi]][i]);
            }
            maskAND = _mm256_and_si256(maskAND, maskOR);
        }
        int r = _mm256_movemask_ps((SIMD_256TYPE)maskAND);
        if (r == 0) {
            continue;
        }
        switch (r) {
            case 0b11:
                selectionVector[svi++] = i * 4;
                break;
            case 0b1100:
                selectionVector[svi++] = i * 4 + 1;
                break;
            case 0b110000:
                selectionVector[svi++] = i * 4 + 2;
                break;
            case 0b11000000:
                selectionVector[svi++] = i * 4 + 3;
                break;
            case 0b1111:
                selectionVector[svi++] = i * 4;
                selectionVector[svi++] = i * 4 + 1;
                break;
            case 0b110011:
                selectionVector[svi++] = i * 4;
                selectionVector[svi++] = i * 4 + 2;
                break;
            case 0b111100:
                selectionVector[svi++] = i * 4 + 1;
                selectionVector[svi++] = i * 4 + 2;
                break;
            case 0b11000011:
                selectionVector[svi++] = i * 4;
                selectionVector[svi++] = i * 4 + 3;
                break;
            case 0b11001100:
                selectionVector[svi++] = i * 4 + 1;
                selectionVector[svi++] = i * 4 + 3;
                break;
            case 0b11110000:
                selectionVector[svi++] = i * 4 + 2;
                selectionVector[svi++] = i * 4 + 3;
                break;
            case 0b111111:
                selectionVector[svi++] = i * 4;
                selectionVector[svi++] = i * 4 + 1;
                selectionVector[svi++] = i * 4 + 2;
                break;
            case 0b11001111:
                selectionVector[svi++] = i * 4;
                selectionVector[svi++] = i * 4 + 1;
                selectionVector[svi++] = i * 4 + 3;
                break;
            case 0b11110011:
                selectionVector[svi++] = i * 4;
                selectionVector[svi++] = i * 4 + 2;
                selectionVector[svi++] = i * 4 + 3;
                break;
            case 0b11111100:
                selectionVector[svi++] = i * 4 + 1;
                selectionVector[svi++] = i * 4 + 2;
                selectionVector[svi++] = i * 4 + 3;
                break;
            case 0b11111111:
                selectionVector[svi++] = i * 4;
                selectionVector[svi++] = i * 4 + 1;
                selectionVector[svi++] = i * 4 + 2;
                selectionVector[svi++] = i * 4 + 3;
                break;
            default:
                break;
        }
        // Create selection vector(SV)
    }
    selVecSize = svi;
}

inline void Executor::vselection() {
    vevalPreds();
    veval();
}

void Executor::projection() {
    if (true) {
        // no operation
        DATA_TYPE buf[VECEX_BYTE];
        for (int si = 0; si < selVecSize; si++) {
            printf("| ");
            for (int k = 0; k < 1; k++) {
                if (keyTypes[k] == DOUBLE) {
                    printf("%lf | ", bufArray[k][selectionVector[si]].d);
                } else if (keyTypes[k] == INT) {
                    printf("%ld | ", bufArray[k][selectionVector[si]].i);
                } else if (keyTypes[k] == TEXT) {
                    buf[sizes[k][si]] = (DATA_TYPE)0;
                    memcpy(buf, &data[indexes[k][si]], sizes[k][si]);
                    printf("%s | ", buf);
                }
            }
            printf("\n");
        }
    } else {
    }
}

void Executor::queryVExec() {
    vmaterialize();
    vselection();
    // projection();
    count += selVecSize;

    // for (int si = 0; si < selVecSize; si += 4) {
    //     // int s = selectionVector[si];
    //     SIMD_128iTYPE sv = _mm_load_epi32(&selectionVector[si]);
    //     SIMD_256iTYPE b = _mm256_i32gather_epi64(bufArray[k], sv, 1);

    //     // proj/aggr
    // }

    tid = 0;
    vtid++;  // debug
}

// void Executor::materialize(int tsize) {
//     for (int s = 0; s < subMatchSize; s++) {
//         // todo: not materialization for key only used in
//         projection/aggregation switch (keyTypes[s]) {
//             case DOUBLE:
//                 for (int ti = 0; ti < tsize; ti++) {
//                     DATA_TYPE *buf = loadBuf(s, ti);
//                     double d = atof((char *)buf);
//                     bufArray[s][ti].d = d;
//                 }
//                 break;
//             case INT:
//                 for (int ti = 0; ti < tsize; ti++) {
//                     DATA_TYPE *buf = loadBuf(s, ti);
//                     int64_t n = (int64_t)atoi((char *)buf);
//                     bufArray[s][ti].i = n;
//                 }
//                 break;
//             case TEXT:
//                 break;
//             default:
//                 break;
//         }
//     }
// }

// void Executor::evalPreds() {
//     for (int sk = 0; sk < numPreds; sk++) {
//         if (predTypes[sk] == DOUBLE) {
//             PredEvaluator<double> prede(predMasks[sk]);
//             prede.evaluate(&predTrees[sk], bufArray);
//         } else if (predTypes[sk] == INT) {
//             PredEvaluator<int64_t> prede(predMasks[sk]);
//             prede.evaluate(&predTrees[sk], bufArray);
//         } else if (predTypes[sk] == TEXT) {
//             PredEvaluator<DATA_TYPE *> prede(predMasks[sk]);
//             prede.evaluateText(&predTrees[sk], data, indexes, sizes);
//         }
//     }
// }

// void Executor::eval(int tsize) {
//     int svi = 0;
//     for (int i = 0; i < tsize; i++) {
//         // todo: design a predicate tree for DNF

//         // initialize mask
//         SIMD_256TYPE maskAND = predMasks[preds[0][0]];
//         for (int oi = 1; oi < predORsize[0]; oi++) {
//             maskAND = _mm256_or_si256(maskAND, predMasks[preds[0][oi]]);
//         }

//         for (int ai = 1; ai < predANDsize; ai++) {
//             SIMD_256TYPE maskOR = predMasks[preds[ai][0]];
//             for (int oi = 1; oi < predORsize[ai]; oi++) {
//                 maskOR = _mm256_or_si256(maskOR, predMasks[preds[ai][oi]]);
//             }
//             maskAND = _mm256_and_si256(maskAND, maskOR);
//         }
//         int r = _mm256_movemask_ps(maskAND);
//         if (r == 0) {
//             continue;
//         }
//         switch (r) {
//             case 1:
//                 selectionVector[svi++] = i * 4;
//                 break;
//             case 2:
//                 selectionVector[svi++] = i * 4 + 1;
//                 break;
//             case 4:
//                 selectionVector[svi++] = i * 4 + 2;
//                 break;
//             case 8:
//                 selectionVector[svi++] = i * 4 + 3;
//                 break;
//             case 3:
//                 selectionVector[svi++] = i * 4;
//                 selectionVector[svi++] = i * 4 + 1;
//                 break;
//             case 5:
//                 selectionVector[svi++] = i * 4;
//                 selectionVector[svi++] = i * 4 + 2;
//                 break;
//             case 6:
//                 selectionVector[svi++] = i * 4 + 1;
//                 selectionVector[svi++] = i * 4 + 2;
//                 break;
//             case 9:
//                 selectionVector[svi++] = i * 4;
//                 selectionVector[svi++] = i * 4 + 3;
//                 break;
//             case 10:
//                 selectionVector[svi++] = i * 4 + 1;
//                 selectionVector[svi++] = i * 4 + 3;
//                 break;
//             case 12:
//                 selectionVector[svi++] = i * 4 + 2;
//                 selectionVector[svi++] = i * 4 + 3;
//                 break;
//             case 7:
//                 selectionVector[svi++] = i * 4;
//                 selectionVector[svi++] = i * 4 + 1;
//                 selectionVector[svi++] = i * 4 + 2;
//                 break;
//             case 11:
//                 selectionVector[svi++] = i * 4;
//                 selectionVector[svi++] = i * 4 + 1;
//                 selectionVector[svi++] = i * 4 + 3;
//                 break;
//             case 13:
//                 selectionVector[svi++] = i * 4;
//                 selectionVector[svi++] = i * 4 + 2;
//                 selectionVector[svi++] = i * 4 + 3;
//                 break;
//             case 14:
//                 selectionVector[svi++] = i * 4 + 1;
//                 selectionVector[svi++] = i * 4 + 2;
//                 selectionVector[svi++] = i * 4 + 3;
//                 break;
//             case 15:
//                 selectionVector[svi++] = i * 4;
//                 selectionVector[svi++] = i * 4 + 1;
//                 selectionVector[svi++] = i * 4 + 2;
//                 selectionVector[svi++] = i * 4 + 3;
//                 break;
//             default:
//                 break;
//         }
//         // Create selection vector(SV)
//     }
//     selVecSize = svi;
// }

// inline void Executor::selection(int tsize) {
//     evalPreds(tsize);
//     eval(tsize);
// }

// void Executor::queryExec(int tsize) {
//     materialize(tsize);
//     selection(tsize);
// }

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

                    i = ctx.recentAcceptIndex + 1;
                }
                ctx.currentState = INIT_STATE;
                break;
        }

        if (acceptStates.find(ctx.currentState) != acceptStates.end()) {
            ctx.recentAcceptState = ctx.currentState;
            ctx.recentAcceptIndex = i;
        }

        if (tid == VECEX_BYTE) {
            queryVExec();
        } else if (i >= size) {
            // queryExec(tid);
            // todo: add non-vectorized execution
            break;
        }
    }

#if (defined BENCH)
    gettimeofday(&lex2, NULL);
    ex_time =
        (lex2.tv_sec - lex1.tv_sec) + (lex2.tv_usec - lex1.tv_usec) * 0.000001;
    printf("#BENCH_exec: %lf s\n\n", ex_time);
#endif

    printf("count: %d\n", count);
    printf("vec count: %d\n", vtid);
}
