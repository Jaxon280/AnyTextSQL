#include "executor/executor.hpp"

namespace vlex {

Executor::Executor() {}

Executor::~Executor() {}

void Executor::setTransTable(const std::vector<Qlabel> &qlabels,
                             int stateSize) {
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
}

void Executor::setSubMatchTable(
    const std::vector<VectFA::SubMatchStates> &subMatchStates, int stateSize) {
    startTable = new int[stateSize]();
    endTable = new BitSet[stateSize]();

    std::set<int> sset;
    for (const VectFA::SubMatchStates &sms : subMatchStates) {
        sset.insert(sms.id);
    }
    subMatchSize = sset.size();
    keyTypes = new Type[subMatchSize];
    for (const VectFA::SubMatchStates &sms : subMatchStates) {
        keyTypes[sms.id] = sms.type;
        for (int ss : sms.startStates) {
            startTable[ss] = sms.id + 1;
        }
        for (int es : sms.states) {
            endTable[es].add(sms.id + 1);
        }
    }
}

void Executor::setVecDatas(const std::vector<Qlabel> &qlabels, int stateSize) {
    SIMDDatas = new SIMD_TEXTTYPE[stateSize];
    SIMDSizes = new int[stateSize]();
    // SIMDCounts = new int[stateSize]();
    int iter = 0;
    for (const Qlabel &it : qlabels) {
        if (it.kind == ORDERED || it.kind == ANY || it.kind == RANGES) {
            DATA_TYPE strdata[16];
            for (int k = 0; k < 16; k++) {
                if (k < (int)it.delta->str.length()) {
                    strdata[k] = (DATA_TYPE)it.delta->str[k];
                } else {
                    strdata[k] = 0;
                }
            }

            SIMDDatas[iter] =
                _mm_loadu_si128(reinterpret_cast<SIMD_TEXTTYPE *>(strdata));
            SIMDSizes[iter] = (int)it.delta->str.size();
        }
        // if (it.kind == ANY_MASK || it.kind == RANGES_MASK) {
        //     SIMDCounts[iter] = it.delta->count;
        // } else {
        //     SIMDCounts[iter] = 0;
        // }
        iter++;
    }
}

void Executor::setFA(VectFA *vfa, SIZE_TYPE _start) {
    ctx.currentState = INIT_STATE;
    ctx.recentAcceptIndex = 0, ctx.recentAcceptState = 0;
    i = _start;

    const std::vector<Qlabel> &qlabels = vfa->getQlabels();
    int stateSize = qlabels.size();
    for (int q = 0; q < stateSize; q++) {
        if (qlabels[q].isAccept) {
            acceptStates.insert(q);
        }
    }

    setTransTable(qlabels, stateSize);
    const std::vector<VectFA::SubMatchStates> &subMatchStates =
        vfa->getSubMatches();
    setSubMatchTable(subMatchStates, stateSize);
    startSMS = new SubMatchNode;
    if (subMatchSize > 0) {
        subMatches = new SubMatch[subMatchSize];
    } else {
        subMatches = NULL;
    }
    setVecDatas(qlabels, stateSize);
}

void Executor::setWildCardProjection() {
    // stmtSize = subMatchSize;
    // stmtList = new Statement[stmtSize];
    // for (int i = 0; i < subMatchSize; i++) {
    //     OpTree *op = new OpTree;
    //     op->evalType = VAR;
    //     op->type;
    //     op->varKeyId = i;
    //     stmtList[i].expr = op;
    //     stmtList[i].httype = NONE_HT;
    //     stmtList[i].name;
    // }
}

void Executor::setStatements(const StatementList *stmts) {
    int stSize = 0;
    for (const StatementList *s = stmts; s != NULL; s = s->next) {
        if (s->stmt->isWildCard && s->stmt->httype == NONE_HT) {
            stSize += subMatchSize;
        } else {
            stSize++;
        }
    }
    stmtSize = stSize;
    stmtList = new Statement[stmtSize];
    aggContexts = new Aggregation[stmtSize];
    int si = 1, agi = 0;
    for (const StatementList *s = stmts; s != NULL; s = s->next) {
        if (s->stmt->isWildCard && s->stmt->httype == NONE_HT) {
            // si += subMatchSize;
            // for (int sii = 1; sii <= subMatchSize; sii++) {
            //     Statement *stmt = new Statement(op, subMatches[sii]);
            //     stmtList[stmtSize - si + sii] = stmt;
            // }
            // httype = NONE_HT;
        } else {
            stmtList[stmtSize - si] = *s->stmt;
            if (s->stmt->httype != NONE_HT) {
                OpTree *op;
                std::queue<OpTree *> bfsQueue;
                bfsQueue.push(s->stmt->expr);
                while (!bfsQueue.empty()) {
                    op = bfsQueue.front();
                    if (op == NULL) {
                        // COUNT(*)
                        aggContexts[agi].type = INT;
                        aggContexts[agi].ftype = COUNT;
                        aggContexts[agi].isWildCard = true;
                        agi++;
                    } else if (op->evalType == OP) {
                        bfsQueue.push(op->left);
                        bfsQueue.push(op->right);
                    } else if (op->evalType == AGGFUNC) {
                        aggContexts[agi].type = op->type;
                        aggContexts[agi].ftype = op->ftype;
                        aggContexts[agi].keyId = op->varKeyId;
                        op->aggId = agi;
                        agi++;
                    }
                    bfsQueue.pop();
                }
            }
            httype = intersectionHTType(s->stmt->httype, httype);
            si++;
        }
    }
    aggSize = agi;
}

void Executor::setSelections(QueryContext *query) {
    ptree = query->getPredTree();
}

void Executor::setAggregations(const std::vector<Key> &gKeyVec) {
    gKeySize = gKeyVec.size();
    gKeys = new Key[gKeySize];
    int ki = 0;
    for (const Key &k : gKeyVec) {
        gKeys[ki].id = k.id;
        gKeys[ki].type = k.type;
        ki++;
    }
    if (gKeySize == 1) {
        aggMap = new AggregationValueMap(aggSize);
        aggCountMap = new AggregationCountMap(aggSize);
    } else if (gKeySize == 0) {
        agg = new data64[aggSize];
        aggCount = new int[aggSize];
        for (int vk = 0; vk < aggSize; vk++) {
            AggFuncType ftype = aggContexts[vk].ftype;
            if (aggContexts[vk].isWildCard) {
                count = 0;
            } else {
                if (aggContexts[vk].type == DOUBLE) {
                    if (ftype == MIN) {
                        agg[vk].d = DBL_MAX;
                    } else if (ftype == MAX) {
                        agg[vk].d = -DBL_MAX;
                    } else {
                        agg[vk].d = 0.0;
                    }
                } else if (aggContexts[vk].type == INT) {
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
    }  // todo: multiple keys
}

void Executor::setQuery(QueryContext *query) {
    tuple = new data64[subMatchSize];
    tsize = new SIZE_TYPE[subMatchSize];

    const StatementList *stmts = query->getStatements();
    setStatements(stmts);
    setSelections(query);
    const std::vector<Key> &gKeyVec = query->getGKeys();
    setAggregations(gKeyVec);
    limit = query->getLimit();
}

void Executor::setSparkContext(SparkContext *sctx, QueryContext *qctx) {
    baseDF = (void *)sctx->ptr;
    curDF = baseDF;
    sizeInRow = sctx->sizeInRow;
    varSize = sctx->varSize, varlens = new int[varSize], varoffsets = new int[varSize];
    for (int ci = 0; ci < varSize; ci++) {
        varlens[ci] = sctx->varCols[ci].size;
        varoffsets[ci] = sctx->varCols[ci].offset;
    }
}

void Executor::cmpestriOrd(ST_TYPE curState) {
loop:
    if (i >= size) return;
    SIMD_TEXTTYPE text =
        _mm_loadu_si128(reinterpret_cast<SIMD_TEXTTYPE *>(&data[i]));
    int r = _mm_cmpestri(SIMDDatas[curState], SIMDSizes[curState], text, 16,
                         _SIDD_CMP_EQUAL_ORDERED);
    if (r == 16) {
        i += 16;
        goto loop;
    }
    i += r;
    if (r > 16 - SIMDSizes[curState]) goto loop;
    i += SIMDSizes[curState];
    if (ctx.currentState == INIT_STATE) {
        resetContext();
    }
    ctx.currentState = rTable[curState];
}

void Executor::cmpestriAny(ST_TYPE curState) {
    if (startTable[curState] > 0) {
        startSubMatch(startTable[curState]);
    }
loop:
    if (i >= size) return;
    SIMD_TEXTTYPE text =
        _mm_loadu_si128(reinterpret_cast<SIMD_TEXTTYPE *>(&data[i]));
    int r = _mm_cmpestri(SIMDDatas[curState], SIMDSizes[curState], text, 16,
                         _SIDD_CMP_EQUAL_ANY);
    if (r == 16) {
        i += 16;
        goto loop;
    }
    i += r;
    if (ctx.currentState == INIT_STATE) {
        resetContext();
    }
    if (i >= size) return;
    ctx.currentState = charTable[curState][(int)data[i++]];
    if (startSMS->id > 0 && !endTable[ctx.currentState].find(startSMS->id)) {
        endSubMatch(startSMS->id);
    }
}

void Executor::cmpestriRanges(ST_TYPE curState) {
    if (startTable[curState] > 0) {
        startSubMatch(startTable[curState]);
    }
loop:
    if (i >= size) return;
    SIMD_TEXTTYPE text =
        _mm_loadu_si128(reinterpret_cast<SIMD_TEXTTYPE *>(&data[i]));
    int r = _mm_cmpestri(SIMDDatas[curState], SIMDSizes[curState], text, 16,
                         _SIDD_CMP_RANGES);
    if (r == 16) {
        i += 16;
        goto loop;
    }
    i += r;
    if (ctx.currentState == INIT_STATE) {
        resetContext();
    }
    if (i >= size) return;
    ctx.currentState = charTable[curState][(int)data[i++]];
    if (startSMS->id > 0 && !endTable[ctx.currentState].find(startSMS->id)) {
        endSubMatch(startSMS->id);
    }
}

// // void Executor::cmpestrmAny(ST_TYPE curState) {
// //     int cnt = 0;
// // loop:
// //     if (i >= size) return;
// //     SIMD_TEXTTYPE text =
// //         _mm_loadu_si128(reinterpret_cast<SIMD_TEXTTYPE *>(&data[i]));
// //     __m128i mm = _mm_cmpestrm(SIMDDatas[curState], SIMDSizes[curState], text, 16,
// //                          _SIDD_CMP_EQUAL_ANY | _SIDD_BIT_MASK);
// //     int m = _mm_extract_epi16(mm, 0);
// //     cnt += __builtin_popcount(m);
// //     if (cnt < SIMDCounts[curState]) {
// //         i += 16;
// //         goto loop;
// //     } else if (cnt > SIMDCounts[curState]) {
// //         int cm;
// //         while (cnt != SIMDCounts[curState]) {
// //             cm = __builtin_ctz(m);
// //             m = m ^ (1 << cm);
// //             cnt--;
// //         }
// //         i += cm;
// //     } else {
// //         i += 31 - __builtin_clz(m);
// //     }

// //     if (charStartTable[ctx.currentState] > 0) {
// //         startSubMatch(charStartTable[ctx.currentState]);
// //     }
// //     if (i >= size) return;
// //     ctx.currentState = charTable[curState][(int)data[i++]];
// //     if (charEndTable[ctx.currentState] > 0) {
// //         endSubMatch(charEndTable[ctx.currentState]);
// //     }
// // }

// // void Executor::cmpestrmRanges(ST_TYPE curState) {
// //     int cnt = 0;
// // loop:
// //     if (i >= size) return;
// //     SIMD_TEXTTYPE text =
// //         _mm_loadu_si128(reinterpret_cast<SIMD_TEXTTYPE *>(&data[i]));
// //     __m128i mm = _mm_cmpestrm(SIMDDatas[curState], SIMDSizes[curState], text, 16,
// //                          _SIDD_CMP_RANGES | _SIDD_BIT_MASK);
// //     int m = _mm_extract_epi16(mm, 0);
// //     cnt += __builtin_popcount(m);
// //     if (cnt < SIMDCounts[curState]) {
// //         i += 16;
// //         goto loop;
// //     } else if (cnt > SIMDCounts[curState]) {
// //         int cm;
// //         while (cnt != SIMDCounts[curState]) {
// //             cm = __builtin_ctz(m);
// //             m = m ^ (1 << cm);
// //             cnt--;
// //         }
// //         i += cm;
// //     } else {
// //         i += 31 - __builtin_clz(m);
// //     }
    
// //     if (charStartTable[ctx.currentState] > 0) {
// //         startSubMatch(charStartTable[ctx.currentState]);
// //     }
// //     if (i >= size) return;
// //     ctx.currentState = charTable[curState][(int)data[i++]];
//     if (charEndTable[ctx.currentState] > 0) {
//         endSubMatch(charEndTable[ctx.currentState]);
//     }
// }

void Executor::startSubMatch(int id) {
    startSMS->id = id;
    startSMS->index = i;
}

void Executor::endSubMatch(int id) {
    subMatches[id - 1].start = startSMS->index;
    subMatches[id - 1].end = i - 1;
    startSMS->id = 0;
}

void Executor::resetContext() {
    ctx.currentState = INIT_STATE, ctx.recentAcceptState = 0,
    ctx.recentAcceptIndex = 0;
    startSMS->id = 0;

    for (int si = 0; si < subMatchSize; si++) {
        subMatches[si].start = UINT64_MAX;
        subMatches[si].end = UINT64_MAX;
    }
}

void Executor::printColumnNames() const {
    printf("|");
    for (int si = 0; si < stmtSize; si++) {
        if (stmtList[si].name == NULL) {
            printf(" %d |", si + 1);
        } else {
            printf(" %s |", stmtList[si].name);
        }
    }
    printf("\n");
}

void Executor::queryStartExec() const { printColumnNames(); }

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
Value Executor::evalFunc1Op(const OpTree *tree, const data64 *vhtv,
                            const int *chtv) const {
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
Value Executor::evalOp(const OpTree *tree) const {
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

bool Executor::evalPred(const OpTree *tree) const {
    if (tree->type == DOUBLE) {
        double lvalue = evalOp<double>(tree->left);
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
        int64_t lvalue = evalOp<int64_t>(tree->left);
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
            return true;
        }
    }
    return false;
}

bool Executor::evalCond(const PredTree *ptree) const {
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
    for (int si = 0; si < stmtSize; si++) {
        OpTree *expr = stmtList[si].expr;
        if (expr->type == DOUBLE) {
            double v = evalOp<double>(expr);
            printf("%lf | ", v);
        } else if (expr->type == INT) {
            int64_t v = evalOp<int64_t>(expr);
            printf("%ld | ", v);
        } else if (expr->type == TEXT) {
            DATA_TYPE buf[128];
            int k = expr->varKeyId;
            buf[tsize[k]] = (DATA_TYPE)0;
            memcpy(buf, tuple[k].p, tsize[k]);
            printf("%s | ", buf);
        }
    }
    printf("\n");
}

void Executor::aggregation0() {
    for (int vk = 0; vk < aggSize; vk++) {
        if (aggContexts[vk].isWildCard) {
            continue;
        }
        int vkk = aggContexts[vk].keyId;
        switch (aggContexts[vk].ftype) {
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
    int kk = gKeys[0].id;

    for (int vk = 0; vk < aggSize; vk++) {
        int vkk = aggContexts[vk].keyId;
        AggFuncType ftype = aggContexts[vk].ftype;
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
        } else if (aggContexts[vk].type == DOUBLE) {
            if (ftype == SUM) {
                std::string s(reinterpret_cast<char *>(tuple[kk].p), tsize[kk]);
                if (!aggMap->find(s)) {
                    aggMap->assign(s, aggContexts);
                }
                aggMap->sumDouble(s, tuple[vkk].d, vk);
            } else if (ftype == AVG) {
                std::string s(reinterpret_cast<char *>(tuple[kk].p), tsize[kk]);
                if (!aggMap->find(s)) {
                    aggMap->assign(s, aggContexts);
                    aggCountMap->assign(s);
                }
                aggMap->sumDouble(s, tuple[vkk].d, vk);
                aggCountMap->count(s, vk);
            } else if (ftype == MAX) {
                std::string s(reinterpret_cast<char *>(tuple[kk].p), tsize[kk]);
                if (!aggMap->find(s)) {
                    aggMap->assign(s, aggContexts);
                }
                aggMap->maxDouble(s, tuple[vkk].d, vk);
            } else if (ftype == MIN) {
                std::string s(reinterpret_cast<char *>(tuple[kk].p), tsize[kk]);
                if (!aggMap->find(s)) {
                    aggMap->assign(s, aggContexts);
                }
                aggMap->minDouble(s, tuple[vkk].d, vk);
            }
        } else if (aggContexts[vk].type == INT) {
            if (ftype == SUM) {
                std::string s(reinterpret_cast<char *>(tuple[kk].p), tsize[kk]);
                if (!aggMap->find(s)) {
                    aggMap->assign(s, aggContexts);
                }
                aggMap->sumInt(s, tuple[vkk].i, vk);
            } else if (ftype == AVG) {
                std::string s(reinterpret_cast<char *>(tuple[kk].p), tsize[kk]);
                if (!aggMap->find(s)) {
                    aggMap->assign(s, aggContexts);
                    aggCountMap->assign(s);
                }
                aggMap->sumInt(s, tuple[vkk].i, vk);
                aggCountMap->count(s, vk);
            } else if (ftype == MAX) {
                std::string s(reinterpret_cast<char *>(tuple[kk].p), tsize[kk]);
                if (!aggMap->find(s)) {
                    aggMap->assign(s, aggContexts);
                }
                aggMap->maxInt(s, tuple[vkk].i, vk);
            } else if (ftype == MIN) {
                std::string s(reinterpret_cast<char *>(tuple[kk].p), tsize[kk]);
                if (!aggMap->find(s)) {
                    aggMap->assign(s, aggContexts);
                }
                aggMap->minInt(s, tuple[vkk].i, vk);
            }
        }
    }
}

void Executor::aggregation() {
    if (gKeySize > 1) {
        // todo: multiple keys
    } else if (gKeySize == 1) {
        aggregation1();
    } else if (gKeySize == 0) {
        aggregation0();
    }
}

void Executor::queryExec() {
    materialize();

    if (selection()) {
        count++;
        if (count <= limit && httype == NONE_HT) {
            projection();
        } else {
            aggregation();
        }
    }
}

void Executor::printAggregation0() const {
    printf("|");
    for (int si = 0; si < stmtSize; si++) {
        if (stmtList[si].isWildCard) {
            printf(" %d |", count);
        } else if (stmtList[si].expr->type == INT) {
            printf(" %ld |", evalOp<int64_t>(stmtList[si].expr));
        } else if (stmtList[si].expr->type == DOUBLE) {
            printf(" %lf |", evalOp<double>(stmtList[si].expr));
        }
    }
    printf("\n");
}

void Executor::printAggregation1() const {
    const AggregationValueMap::HTType &vht = aggMap->getHashTable();
    const AggregationCountMap::HTType &cht = aggCountMap->getHashTable();
    auto vbegin = vht.cbegin();
    auto vend = vht.cbegin();
    auto cbegin = cht.cbegin();
    auto cend = cht.cbegin();

    if (httype == COUNT_HT) {
        if (limit && limit <= (int)cht.size()) {
            for (int i = 0; i < limit; i++) {
                ++cend;
            }
        } else {
            cend = cht.cend();
        }
    } else if (httype == VALUE_HT || httype == BOTH_HT) {
        if (limit && limit <= (int)vht.size()) {
            for (int i = 0; i < limit; i++) {
                ++vend;
            }
        } else {
            vend = vht.cend();
        }
    }

    if (httype == COUNT_HT) {
        for (auto it = cbegin; it != cend; ++it) {
            printf("|");
            for (int si = 0; si < stmtSize; si++) {
                if (stmtList[si].httype == NONE_HT) {
                    printf(" %s |", it->first.c_str());
                } else {
                    if (stmtList[si].expr->type == INT) {
                        printf(" %ld |",
                               evalFunc1Op<int64_t>(stmtList[si].expr, NULL,
                                                    cht.at(it->first)));
                    } else if (stmtList[si].expr->type == DOUBLE) {
                        printf(" %lf |",
                               evalFunc1Op<double>(stmtList[si].expr, NULL,
                                                   cht.at(it->first)));
                    }
                }
            }
            printf("\n");
        }
    } else if (httype == VALUE_HT || httype == BOTH_HT) {
        for (auto it = vbegin; it != vend; ++it) {
            printf("|");
            for (int si = 0; si < stmtSize; si++) {
                if (stmtList[si].httype == NONE_HT) {
                    printf(" %s |", it->first.c_str());
                } else {
                    if (stmtList[si].expr->type == INT) {
                        int64_t avi = 0;
                        if (stmtList[si].httype == COUNT_HT) {
                            avi = evalFunc1Op<int64_t>(stmtList[si].expr, NULL,
                                                       cht.at(it->first));
                        } else if (stmtList[si].httype == VALUE_HT) {
                            avi = evalFunc1Op<int64_t>(stmtList[si].expr,
                                                       vht.at(it->first), NULL);
                        } else if (stmtList[si].httype == BOTH_HT) {
                            avi = evalFunc1Op<int64_t>(stmtList[si].expr,
                                                       vht.at(it->first),
                                                       cht.at(it->first));
                        }
                        printf(" %ld |", avi);
                    } else if (stmtList[si].expr->type == DOUBLE) {
                        double avd = 0.0;
                        if (stmtList[si].httype == COUNT_HT) {
                            avd = evalFunc1Op<double>(stmtList[si].expr, NULL,
                                                      cht.at(it->first));
                        } else if (stmtList[si].httype == VALUE_HT) {
                            avd = evalFunc1Op<double>(stmtList[si].expr,
                                                      vht.at(it->first), NULL);
                        } else if (stmtList[si].httype == BOTH_HT) {
                            avd = evalFunc1Op<double>(stmtList[si].expr,
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

void Executor::queryEndExec() const {
    if (httype == NONE_HT) {
    } else {
        if (gKeySize > 1) {
        } else if (gKeySize == 1) {
            printAggregation1();
        } else if (gKeySize == 0) {
            printAggregation0();
        }
    }
}

// follow memory layout of UnsafeRow
void Executor::storeToDataFrame() {
    // 1. Null bit (8bytes)
    // 2. Add String/Int/Double flags
    // 3. Add Varlen data
    
    int offset = 0;
    int ti = 0;
    memset(curDF, 0, 8); // TODO: add Null variance.
    offset += 8;
    DATA_TYPE buf[128];
    double d;
    long long l;

    int ssize;
    for (int sii = 0; sii < subMatchSize; sii++) {
        SIZE_TYPE start = subMatches[sii].start;
        SIZE_TYPE end = subMatches[sii].end;
        if (start == UINT64_MAX || end == UINT64_MAX) continue;
        SIZE_TYPE size = end - start;
        switch (keyTypes[sii]) {
            case DOUBLE:
                memcpy(buf, &data[start], size);
                buf[size] = (DATA_TYPE)0;
                d = atof((char *)buf);
                memcpy(curDF + offset, &d, sizeof(double));
                break;
            case INT:
                memcpy(buf, &data[start], size);
                buf[size] = (DATA_TYPE)0;
                l = atoll((char *)buf);
                memcpy(curDF + offset, &l, sizeof(long long));
                break;
            case TEXT:
                ssize = size > varlens[ti] ? varlens[ti] : size;
                memcpy(curDF + offset, &ssize , sizeof(int)); // size part
                memcpy(curDF + offset + sizeof(int), &varoffsets[ti], sizeof(int)); // offset part
                memcpy(curDF + varoffsets[ti], &data[start], ssize);
                ti++;
                break;
            default:
                break;
        }
        offset += 8;
    }

    curDF += sizeInRow;
    rowCount++;
}

void Executor::preStoreToDataFrame() {
    int offset = 0;
    int ti = 0;
    memset(curDF, 0, 8); // TODO: add Null variance.
    offset += 8;
    DATA_TYPE buf[256];
    double d;
    long long l;

    int ssize;
    for (int sii = 0; sii < subMatchSize; sii++) {
        SIZE_TYPE start = subMatches[sii].start;
        SIZE_TYPE end = subMatches[sii].end;
        if (start == UINT64_MAX || end == UINT64_MAX) continue;

        if (start >= prevStart) {
            if (end >= prevStart) {
                size = end - start;
                memcpy(buf, &prevData[start - prevStart], size);
            } else {
                SIZE_TYPE size1 = prevSize - (start - prevStart);
                SIZE_TYPE size2 = end;
                size = (size1 + size2);
                memcpy(buf, &prevData[start - prevStart], size1);
                memcpy(buf + size1, data, size2);
            }
        } else {
            if (end >= prevStart) {
                continue;
            } else {
                size = end - start;
                memcpy(buf, &data[start], size);
            }
        }
        buf[size] = (DATA_TYPE)0;

        switch (keyTypes[sii]) {
            case DOUBLE:
                d = atof((char *)buf);
                memcpy(curDF + offset, &d, sizeof(double));
                break;
            case INT:
                l = atoll((char *)buf);
                memcpy(curDF + offset, &l, sizeof(long long));
                break;
            case TEXT:
                ssize = size > varlens[ti] ? varlens[ti] : size;
                memcpy(curDF + offset, &ssize , sizeof(int)); // size part
                memcpy(curDF + offset + sizeof(int), &varoffsets[ti], sizeof(int)); // offset part
                memcpy(curDF + varoffsets[ti], buf, ssize);
                ti++;
                break;
            default:
                break;
        }
        offset += 8;
    }

    curDF += sizeInRow;
    rowCount++;
}

SIZE_TYPE Executor::preExec(DATA_TYPE *_data, SIZE_TYPE _size) {
    if (prevData == NULL) { return 0; }

    data = _data, _size = size, i = 0;
    while (i < size) {
        switch (kindTable[ctx.currentState]) {
            case ORDERED:
                cmpestriOrd(ctx.currentState);
                break;
            case ANY:
                cmpestriAny(ctx.currentState);
                break;
            case RANGES:
                cmpestriRanges(ctx.currentState);
                break;
            case C:
                if (ctx.currentState == INIT_STATE) {
                    resetContext();
                }

                if (startTable[ctx.currentState] > 0) {
                    startSubMatch(startTable[ctx.currentState]);
                }
                ctx.currentState = charTable[ctx.currentState][(int)data[i++]];
                if (startSMS->id > 0 && !endTable[ctx.currentState].find(startSMS->id)) {
                    endSubMatch(startSMS->id);
                }
                break;
            default:
                if (ctx.recentAcceptState) {
                    if (startSMS->id > 0) {
                        subMatches[startSMS->id - 1].start = startSMS->index;
                        subMatches[startSMS->id - 1].end = i - 1;
                        startSMS->id = 0;
                    }
                    // preStoreToDataFrame();
                    if (ctx.recentAcceptIndex > prevStart) {
                        return 0;
                    } else {
                        SIZE_TYPE s = ctx.recentAcceptIndex + 1;
                        return s;
                    }
                }
                resetContext();
                return 0;
        }

        if (acceptStates.find(ctx.currentState) != acceptStates.end()) {
            ctx.recentAcceptState = ctx.currentState;
            ctx.recentAcceptIndex = i;
        }
    }

    return size;
}

void Executor::postExec() {
    if (prevData != NULL) delete prevData;
    SIZE_TYPE start = UINT64_MAX;
    for (int si = 0; si < subMatchSize; si++) {
        if (start > subMatches[si].start && subMatches[si].start != UINT64_MAX) {
            start = subMatches[si].start;
        }
    }

    if (start == UINT64_MAX) {
        prevData = NULL;
        return;
    }
    prevStart = start;
    prevSize = size - prevStart;
    prevData = new DATA_TYPE[prevSize];

    // copy;
    for (SIZE_TYPE ii = 0; ii < prevSize; ii++) {
        prevData[ii] = data[ii + prevStart];
    }
}

void Executor::exec(DATA_TYPE *_data, SIZE_TYPE _size, SIZE_TYPE start) {
    data = _data, size = _size, i = start;

#if (defined BENCH)
    double ex_time = 0.0;
    timeval lex1, lex2;
    gettimeofday(&lex1, NULL);
#endif
    queryStartExec();

    while (i < size) {
        switch (kindTable[ctx.currentState]) {
            case ORDERED:
                cmpestriOrd(ctx.currentState);
                break;
            case ANY:
                cmpestriAny(ctx.currentState);
                break;
            case RANGES:
                cmpestriRanges(ctx.currentState);
                break;
            // case ANY_MASK:
            //     cmpestrmAny(ctx.currentState);
            //     break;
            // case RANGES_MASK:
            //     cmpestrmRanges(ctx.currentState);
            //     break;
            case C:
                if (ctx.currentState == INIT_STATE) {
                    resetContext();
                }

                if (startTable[ctx.currentState] > 0) {
                    startSubMatch(startTable[ctx.currentState]);
                }
                ctx.currentState = charTable[ctx.currentState][(int)data[i++]];
                if (startSMS->id > 0 && !endTable[ctx.currentState].find(startSMS->id)) {
                    endSubMatch(startSMS->id);
                }
                break;
            default:
                if (ctx.recentAcceptState) {
                    // todo: change it according to query plan
                    if (startSMS->id > 0) {
                        subMatches[startSMS->id - 1].start = startSMS->index;
                        subMatches[startSMS->id - 1].end = i - 1;
                        startSMS->id = 0;
                    }
#if (defined VECEXEC)
                    qexec->materialize(tid);
                    tid++;
                    qexec->queryVExec();
#else
                    queryExec();
#endif

                    i = ctx.recentAcceptIndex + 1;
                }
                resetContext();
                break;
        }

        if (acceptStates.find(ctx.currentState) != acceptStates.end()) {
            ctx.recentAcceptState = ctx.currentState;
            ctx.recentAcceptIndex = i;
        }
    }

    queryEndExec();

#if (defined BENCH)
    gettimeofday(&lex2, NULL);
    ex_time =
        (lex2.tv_sec - lex1.tv_sec) + (lex2.tv_usec - lex1.tv_usec) * 0.000001;
    printf("#BENCH_exec: %lf s\n\n", ex_time);
#endif
}

int Executor::execWithSpark(DATA_TYPE *_data, SIZE_TYPE _size, SIZE_TYPE start) {
    data = _data, size = _size, i = start;

#if (defined BENCH)
    double ex_time = 0.0;
    timeval lex1, lex2;
    gettimeofday(&lex1, NULL);
#endif

    while (i < size) {
        switch (kindTable[ctx.currentState]) {
            case ORDERED:
                cmpestriOrd(ctx.currentState);
                break;
            case ANY:
                cmpestriAny(ctx.currentState);
                break;
            case RANGES:
                cmpestriRanges(ctx.currentState);
                break;
            // case ANY_MASK:
            //     cmpestrmAny(ctx.currentState);
            //     break;
            // case RANGES_MASK:
            //     cmpestrmRanges(ctx.currentState);
            //     break;
            case C:
                if (ctx.currentState == INIT_STATE) {
                    resetContext();
                }

                if (startTable[ctx.currentState] > 0) {
                    startSubMatch(startTable[ctx.currentState]);
                }
                ctx.currentState = charTable[ctx.currentState][(int)data[i++]];
                if (startSMS->id > 0 && !endTable[ctx.currentState].find(startSMS->id)) {
                    endSubMatch(startSMS->id);
                }
                break;
            default:
                if (ctx.recentAcceptState) {
                    if (startSMS->id > 0) {
                        subMatches[startSMS->id - 1].start = startSMS->index;
                        subMatches[startSMS->id - 1].end = i - 1;
                        startSMS->id = 0;
                    }
                    storeToDataFrame();
                    i = ctx.recentAcceptIndex + 1;
                }
                resetContext();
                break;
        }

        if (acceptStates.find(ctx.currentState) != acceptStates.end()) {
            ctx.recentAcceptState = ctx.currentState;
            ctx.recentAcceptIndex = i;
        }
    }

#if (defined BENCH)
    gettimeofday(&lex2, NULL);
    ex_time =
        (lex2.tv_sec - lex1.tv_sec) + (lex2.tv_usec - lex1.tv_usec) * 0.000001;
    printf("#BENCH_exec (%d): %lf s\n\n", chunk, ex_time);
    execTotal += ex_time;
    printf("Cumulative #BENCH_exec (%d): %lf s\n\n", chunk, execTotal);
    chunk++;
#endif

    return rowCount;
}
}  // namespace vlex
