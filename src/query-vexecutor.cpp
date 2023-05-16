#include "query-engine/query-vexecutor.hpp"

namespace vlex {

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

void QueryVExecutor::materialize(int tid) {
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

void QueryVExecutor::vmaterialize() {
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

void QueryVExecutor::vevalPreds() {
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

void QueryVExecutor::veval() {
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

void QueryVExecutor::vprojection() {
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

void QueryVExecutor::vaggregation0() {
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

inline void QueryVExecutor::vaggregation() {
    int aggSize = aggContext->keys.size();
    if (aggSize > 1) {
        // todo: multiple keys
    } else if (aggSize == 1) {
        vaggregation1();
    } else {
        vaggregation0();
    }
}

inline void QueryVExecutor::queryVExec() {
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

}  // namespace vlex
