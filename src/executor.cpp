#include "executor/executor.hpp"

namespace vlex {

Executor::Executor() {}
Executor::Executor(VectFA *vfa, QueryContext *query, SIZE_TYPE _start) {
    setVFA(vfa, _start);
    setQuery(query);
}

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
    anyStartTable = new int[stateSize]();
    anyEndTable = new int[stateSize]();
    charStartTable = new int[stateSize]();
    charEndTable = new int[stateSize]();
    endPredIdTable = new int[stateSize]();

    subMatchSize = subMatchStates.size();
    keyTypes = new Type[subMatchSize];
    for (const VectFA::SubMatchStates &sms : subMatchStates) {
        keyTypes[sms.id] = sms.type;
        // sms.predID
        for (int ss : sms.charStartStates) {
            charStartTable[ss] = sms.id + 1;
        }
        for (int es : sms.charEndStates) {
            charEndTable[es] = sms.id + 1;
            endPredIdTable[es] = sms.predID;
        }
        for (int ss : sms.anyStartStates) {
            anyStartTable[ss] = sms.id + 1;
        }
        for (int es : sms.anyEndStates) {
            anyEndTable[es] = sms.id + 1;
            endPredIdTable[es] = sms.predID;
        }
    }
}

void Executor::setVecDatas(const std::vector<Qlabel> &qlabels, int stateSize) {
    SIMDDatas = new SIMD_TEXTTYPE[stateSize];
    SIMDSizes = new int[stateSize];
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
        iter++;
    }
}

void Executor::setVFA(VectFA *vfa, SIZE_TYPE _start) {
    ctx.currentState = INIT_STATE;
    ctx.tokenStartIndex = _start, ctx.recentAcceptIndex = 0,
    ctx.recentAcceptState = 0;
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
    end = new SubMatchNode;
    if (subMatchSize > 0) {
        subMatches = new SubMatch[subMatchSize];
    } else {
        subMatches = NULL;
    }
    setVecDatas(qlabels, stateSize);
}

void Executor::setStatements(const StatementList *stmts) {
    int stSize = 0;
    for (const StatementList *s = stmts; s != NULL; s = s->next) {
        stSize++;
    }
    stmtSize = stSize;
    stmtList = new Statement[stmtSize];
    aggContexts = new Aggregation[stmtSize];
    int si = 1, agi = 0;
    for (const StatementList *s = stmts; s != NULL; s = s->next) {
        stmtList[stmtSize - si] = *s->stmt;
        if (s->stmt->httype != NONE_HT) {
            OpTree *op;
            std::queue<OpTree *> bfsQueue;
            bfsQueue.push(s->stmt->expr);
            while (!bfsQueue.empty()) {
                op = bfsQueue.front();
                if (op->evalType == OP) {
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
    aggSize = agi;
}

void Executor::setSelections(QueryContext *query) {
    ptree = query->getPredTree();
    textPredNum = query->getTextPredNum();
    textPredResults = new int[textPredNum + 1];
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

inline void Executor::cmpestriOrd(ST_TYPE cur_state) {
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

inline void Executor::cmpestriAny(ST_TYPE cur_state) {
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

inline void Executor::cmpestriRanges(ST_TYPE cur_state) {
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

void Executor::printAggregation0() const {
    printf("|");
    for (int si = 0; si < stmtSize; si++) {
        if (stmtList[si].expr->type == INT) {
            printf(" %ld |", evalOp<int64_t>(stmtList[si].expr));
        } else if (stmtList[si].expr->type == DOUBLE) {
            printf(" %lf |", evalOp<double>(stmtList[si].expr));
        } else {
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
#if (defined VECEXEC)
                    qexec->materialize(tid);
                    tid++;
                    qexec->queryVExec();
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

    queryEndExec();

#if (defined BENCH)
    gettimeofday(&lex2, NULL);
    ex_time =
        (lex2.tv_sec - lex1.tv_sec) + (lex2.tv_usec - lex1.tv_usec) * 0.000001;
    printf("#BENCH_exec: %lf s\n\n", ex_time);
#endif
}
}  // namespace vlex
