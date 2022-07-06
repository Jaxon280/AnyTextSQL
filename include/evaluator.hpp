#pragma once

#include "common.hpp"
#include "query.hpp"

namespace vlex {
using namespace vlex;

template <typename Value>
class OpVEvaluator {
    Value *data;

    void evalAdd(Value *lvalue, Value *rvalue, Value *output) {
        if constexpr (std::is_same_v<Value, double>) {
            for (int i = 0; i < VECEX_BYTE; i += 8) {
                SIMD_512dTYPE vl = _mm512_load_pd(&lvalue[i]);
                SIMD_512dTYPE vr = _mm512_load_pd(&rvalue[i]);
                SIMD_512dTYPE v = _mm512_add_pd(vl, vr);
                _mm512_store_pd(&output[i], v);
            }
        } else if constexpr (std::is_same_v<Value, int64_t>) {
            for (int i = 0; i < VECEX_BYTE; i += 8) {
                SIMD_512iTYPE vl = _mm512_load_epi64(&lvalue[i]);
                SIMD_512iTYPE vr = _mm512_load_epi64(&rvalue[i]);
                SIMD_512iTYPE v = _mm512_add_epi64(vl, vr);
                _mm512_store_epi64(&output[i], v);
            }
        }
    }

    void evalSub(Value *lvalue, Value *rvalue, Value *output) {
        if constexpr (std::is_same_v<Value, double>) {
            for (int i = 0; i < VECEX_BYTE; i += 8) {
                SIMD_512dTYPE vl = _mm512_load_pd(&lvalue[i]);
                SIMD_512dTYPE vr = _mm512_load_pd(&rvalue[i]);
                SIMD_512dTYPE v = _mm512_sub_pd(vl, vr);
                _mm512_store_pd(&output[i], v);
            }
        } else if constexpr (std::is_same_v<Value, int64_t>) {
            for (int i = 0; i < VECEX_BYTE; i += 8) {
                SIMD_512iTYPE vl = _mm512_load_epi64(&lvalue[i]);
                SIMD_512iTYPE vr = _mm512_load_epi64(&rvalue[i]);
                SIMD_512iTYPE v = _mm512_sub_epi64(vl, vr);
                _mm512_store_epi64(&output[i], v);
            }
        }
    }

    void evalMul(Value *lvalue, Value *rvalue, Value *output) {
        if constexpr (std::is_same_v<Value, double>) {
            for (int i = 0; i < VECEX_BYTE; i += 8) {
                SIMD_512dTYPE vl = _mm512_load_pd(&lvalue[i]);
                SIMD_512dTYPE vr = _mm512_load_pd(&rvalue[i]);
                SIMD_512dTYPE v = _mm512_mul_pd(vl, vr);
                _mm512_store_pd(&output[i], v);
            }
        } else if constexpr (std::is_same_v<Value, int64_t>) {
            // for (int i = 0; i < VECEX_BYTE; i += 8) {
            //     SIMD_512iTYPE vl = _mm512_load_epi64(&lvalue[i]);
            //     SIMD_512iTYPE vr = _mm512_load_epi64(&rvalue[i]);
            //     SIMD_512iTYPE v = _mm512_mul_epi64(vl, vr);
            //     _mm512_store_epi64(&output[i], v);
            // }

            // todo: implement mul
        }
    }

    void evalDiv(Value *lvalue, Value *rvalue, Value *output) {
        if constexpr (std::is_same_v<Value, double>) {
            for (int i = 0; i < VECEX_BYTE; i += 8) {
                SIMD_512dTYPE vl = _mm512_load_pd(&lvalue[i]);
                SIMD_512dTYPE vr = _mm512_load_pd(&rvalue[i]);
                SIMD_512dTYPE v = _mm512_div_pd(vl, vr);
                _mm512_store_pd(&output[i], v);
            }
        } else if constexpr (std::is_same_v<Value, int64_t>) {
            // for (int i = 0; i < VECEX_BYTE; i += 8) {
            //     SIMD_512iTYPE vl = _mm512_load_epi64(&lvalue[i]);
            //     SIMD_512iTYPE vr = _mm512_load_epi64(&rvalue[i]);
            //     SIMD_512iTYPE v = _mm512_add_epi64(vl, vr);
            //     _mm512_store_epi64(&output[i], v);
            // }

            // todo: implement div
        }
    }

    void evalAddConst(Value *lvalue, Value rvalue, Value *output) {
        if constexpr (std::is_same_v<Value, double>) {
            SIMD_512dTYPE cr = _mm512_set1_pd(rvalue);
            for (int i = 0; i < VECEX_BYTE; i += 8) {
                SIMD_512dTYPE vl = _mm512_load_pd(&lvalue[i]);
                SIMD_512dTYPE v = _mm512_add_pd(vl, cr);
                _mm512_store_pd(&output[i], v);
            }
        } else if constexpr (std::is_same_v<Value, int64_t>) {
            SIMD_512iTYPE cr = _mm512_set1_epi64(rvalue);
            for (int i = 0; i < VECEX_BYTE; i += 8) {
                SIMD_512iTYPE vl = _mm512_load_epi64(&lvalue[i]);
                SIMD_512iTYPE v = _mm512_add_epi64(vl, cr);
                _mm512_store_epi64(&output[i], v);
            }
        }
    }

    void evalSubConst(Value *lvalue, Value rvalue, Value *output) {
        if constexpr (std::is_same_v<Value, double>) {
            SIMD_512dTYPE cr = _mm512_set1_pd(rvalue);
            for (int i = 0; i < VECEX_BYTE; i += 8) {
                SIMD_512dTYPE vl = _mm512_load_pd(&lvalue[i]);
                SIMD_512dTYPE v = _mm512_add_pd(vl, cr);
                _mm512_store_pd(&output[i], v);
            }
        } else if constexpr (std::is_same_v<Value, int64_t>) {
            SIMD_512iTYPE cr = _mm512_set1_epi64(rvalue);
            for (int i = 0; i < VECEX_BYTE; i += 8) {
                SIMD_512iTYPE vl = _mm512_load_epi64(&lvalue[i]);
                SIMD_512iTYPE v = _mm512_sub_epi64(vl, cr);
                _mm512_store_epi64(&output[i], v);
            }
        }
    }

    void evalMulConst(Value *lvalue, Value rvalue, Value *output) {
        if constexpr (std::is_same_v<Value, double>) {
            SIMD_512dTYPE cr = _mm512_set1_pd(rvalue);
            for (int i = 0; i < VECEX_BYTE; i += 8) {
                SIMD_512dTYPE vl = _mm512_load_pd(&lvalue[i]);
                SIMD_512dTYPE v = _mm512_mul_pd(vl, cr);
                _mm512_store_pd(&output[i], v);
            }
        } else if constexpr (std::is_same_v<Value, int64_t>) {
            // SIMD_512iTYPE cr = _mm512_set1_epi64(rvalue);
            // for (int i = 0; i < VECEX_BYTE; i += 8) {
            //     SIMD_512iTYPE vl = _mm512_load_epi64(&lvalue[i]);
            //     SIMD_512iTYPE v = _mm512_add_epi64(vl, cr);
            //     _mm512_store_epi64(&output[i], v);
            // }

            // todo: implement mul
        }
    }

    void evalDivConst(Value *lvalue, Value rvalue, Value *output) {
        if constexpr (std::is_same_v<Value, double>) {
            SIMD_512dTYPE cr = _mm512_set1_pd(rvalue);
            for (int i = 0; i < VECEX_BYTE; i += 8) {
                SIMD_512dTYPE vl = _mm512_load_pd(&lvalue[i]);
                SIMD_512dTYPE v = _mm512_div_pd(vl, cr);
                _mm512_store_pd(&output[i], v);
            }
        } else if constexpr (std::is_same_v<Value, int64_t>) {
            // SIMD_512iTYPE cr = _mm512_set1_epi64(rvalue);
            // for (int i = 0; i < VECEX_BYTE; i += 8) {
            //     SIMD_512iTYPE vl = _mm512_load_epi64(&lvalue[i]);
            //     SIMD_512iTYPE v = _mm512_add_epi64(vl, cr);
            //     _mm512_store_epi64(&output[i], v);
            // }

            // implement div
        }
    }

    void evalVar(Value *left, Value *right, OpType op) {
        switch (op) {
            case ADD:
                evalAdd(left, right, data);
                break;
            case SUB:
                evalSub(left, right, data);
                break;
            case MUL:
                evalMul(left, right, data);
                break;
            case DIV:
                evalDiv(left, right, data);
                break;
            default:
                break;
        }
    }

    void evalConst(Value *left, Value right, OpType op) {
        switch (op) {
            case ADD:
                evalAddConst(left, right, data);
                break;
            case SUB:
                evalSubConst(left, right, data);
                break;
            case MUL:
                evalMulConst(left, right, data);
                break;
            case DIV:
                evalDivConst(left, right, data);
                break;
            default:
                break;
        }
    }

   public:
    OpVEvaluator() {
        data = new (std::align_val_t{VECEX_SIZE}) Value[VECEX_BYTE];
    }
    ~OpVEvaluator() { delete data; }

    Value *evaluate(QueryContext::OpTree *tree, data64 **bufArray) {
        Value *lvalue;
        if (tree->left->evalType == OP) {
            if (tree->left->type == DOUBLE) {
                OpVEvaluator<double> ope1;
                lvalue = (Value *)ope1.evaluate(tree->left, bufArray);
            } else if (tree->left->type == INT) {
                OpVEvaluator<int64_t> ope1;
                lvalue = (Value *)ope1.evaluate(tree->left, bufArray);
            }
        } else if (tree->left->evalType == VAR) {
            lvalue = reinterpret_cast<Value *>(bufArray[tree->left->varKey]);
        }

        Value *rvalue;
        if (tree->right->evalType == OP) {
            if (tree->right->type == DOUBLE) {
                OpVEvaluator<double> ope2;
                rvalue = (Value *)ope2.evaluate(tree->right, bufArray);
            } else if (tree->right->type == INT) {
                OpVEvaluator<int64_t> ope2;
                rvalue = (Value *)ope2.evaluate(tree->right, bufArray);
            }
            evalVar(lvalue, rvalue, tree->opType);
        } else if (tree->right->evalType == VAR) {
            rvalue = reinterpret_cast<Value *>(bufArray[tree->right->varKey]);
            evalVar(lvalue, rvalue, tree->opType);
        } else if (tree->right->evalType == CONST) {
            if (tree->type == DOUBLE) {
                evalConst(lvalue, tree->right->constData.d, tree->opType);
            } else if (tree->type == INT) {
                evalConst(lvalue, tree->right->constData.i, tree->opType);
            }
        }
        return data;
    }
};

template <typename Value>
class PredVEvaluator {
    uint8_t *masks;

    void evalVar(Value *left, Value *right, OpType op) {
        if constexpr (std::is_same_v<Value, double>) {
            switch (op) {
                case EQUAL:
                    for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                        SIMD_512dTYPE vl = _mm512_load_pd(&left[ti << 3]);
                        SIMD_512dTYPE vr = _mm512_load_pd(&right[ti << 3]);
                        masks[ti] = _mm512_cmp_pd_mask(vl, vr, _CMP_EQ_OQ);
                    }
                    break;
                case NEQUAL:
                    for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                        SIMD_512dTYPE vl = _mm512_load_pd(&left[ti << 3]);
                        SIMD_512dTYPE vr = _mm512_load_pd(&right[ti << 3]);
                        masks[ti] = _mm512_cmp_pd_mask(vl, vr, _CMP_NEQ_OQ);
                    }
                    break;
                case LESS:
                    for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                        SIMD_512dTYPE vl = _mm512_load_pd(&left[ti << 3]);
                        SIMD_512dTYPE vr = _mm512_load_pd(&right[ti << 3]);
                        masks[ti] = _mm512_cmp_pd_mask(vl, vr, _CMP_LT_OQ);
                    }
                    break;
                case LESSEQ:
                    for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                        SIMD_512dTYPE vl = _mm512_load_pd(&left[ti << 3]);
                        SIMD_512dTYPE vr = _mm512_load_pd(&right[ti << 3]);
                        masks[ti] = _mm512_cmp_pd_mask(vl, vr, _CMP_LE_OQ);
                    }
                    break;
                case GREATEQ:
                    for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                        SIMD_512dTYPE vl = _mm512_load_pd(&left[ti << 3]);
                        SIMD_512dTYPE vr = _mm512_load_pd(&right[ti << 3]);
                        masks[ti] = _mm512_cmp_pd_mask(vl, vr, _CMP_GE_OQ);
                    }
                    break;
                case GREATER:
                    for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                        SIMD_512dTYPE vl = _mm512_load_pd(&left[ti << 3]);
                        SIMD_512dTYPE vr = _mm512_load_pd(&right[ti << 3]);
                        masks[ti] = _mm512_cmp_pd_mask(vl, vr, _CMP_GT_OQ);
                    }
                    break;
                default:
                    break;
            }
        } else if constexpr (std::is_same_v<Value, int64_t>) {
            switch (op) {
                case EQUAL:
                    for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                        SIMD_512iTYPE vl = _mm512_load_epi64(&left[ti << 3]);
                        SIMD_512iTYPE vr = _mm512_load_epi64(&right[ti << 3]);
                        masks[ti] = _mm512_cmpeq_epi64_mask(vl, vr);
                    }
                    break;
                case NEQUAL:
                    for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                        SIMD_512iTYPE vl = _mm512_load_epi64(&left[ti << 3]);
                        SIMD_512iTYPE vr = _mm512_load_epi64(&right[ti << 3]);
                        masks[ti] = _mm512_cmpneq_epi64_mask(vl, vr);
                    }
                    break;
                case LESS:
                    for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                        SIMD_512iTYPE vl = _mm512_load_epi64(&left[ti << 3]);
                        SIMD_512iTYPE vr = _mm512_load_epi64(&right[ti << 3]);
                        masks[ti] = _mm512_cmplt_epi64_mask(vl, vr);
                    }
                    break;
                case LESSEQ:
                    for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                        SIMD_512iTYPE vl = _mm512_load_epi64(&left[ti << 3]);
                        SIMD_512iTYPE vr = _mm512_load_epi64(&right[ti << 3]);
                        masks[ti] = _mm512_cmple_epi64_mask(vl, vr);
                    }
                    break;
                case GREATEQ:
                    for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                        SIMD_512iTYPE vl = _mm512_load_epi64(&left[ti << 3]);
                        SIMD_512iTYPE vr = _mm512_load_epi64(&right[ti << 3]);
                        masks[ti] = _mm512_cmpge_epi64_mask(vl, vr);
                    }
                    break;
                case GREATER:
                    for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                        SIMD_512iTYPE vl = _mm512_load_epi64(&left[ti << 3]);
                        SIMD_512iTYPE vr = _mm512_load_epi64(&right[ti << 3]);
                        masks[ti] = _mm512_cmpgt_epi64_mask(vl, vr);
                    }
                    break;
                default:
                    break;
            }
        }
    }

    void evalConst(Value *left, Value right, OpType op) {
        if constexpr (std::is_same_v<Value, double>) {
            SIMD_512dTYPE cr = _mm512_set1_pd(right);
            switch (op) {
                case EQUAL:
                    for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                        SIMD_512dTYPE vl = _mm512_load_pd(&left[ti << 3]);
                        masks[ti] = _mm512_cmp_pd_mask(vl, cr, _CMP_EQ_OQ);
                    }
                    break;
                case NEQUAL:
                    for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                        SIMD_512dTYPE vl = _mm512_load_pd(&left[ti << 3]);
                        masks[ti] = _mm512_cmp_pd_mask(vl, cr, _CMP_NEQ_OQ);
                    }
                    break;
                case LESS:
                    for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                        SIMD_512dTYPE vl = _mm512_load_pd(&left[ti << 3]);
                        masks[ti] = _mm512_cmp_pd_mask(vl, cr, _CMP_LT_OQ);
                    }
                    break;
                case LESSEQ:
                    for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                        SIMD_512dTYPE vl = _mm512_load_pd(&left[ti << 3]);
                        masks[ti] = _mm512_cmp_pd_mask(vl, cr, _CMP_LE_OQ);
                    }
                    break;
                case GREATEQ:
                    for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                        SIMD_512dTYPE vl = _mm512_load_pd(&left[ti << 3]);
                        masks[ti] = _mm512_cmp_pd_mask(vl, cr, _CMP_GE_OQ);
                    }
                    break;
                case GREATER:
                    for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                        SIMD_512dTYPE vl = _mm512_load_pd(&left[ti << 3]);
                        masks[ti] = _mm512_cmp_pd_mask(vl, cr, _CMP_GT_OQ);
                    }
                    break;
            }
        } else if constexpr (std::is_same_v<Value, int64_t>) {
            SIMD_512iTYPE cr = _mm512_set1_epi64(right);
            switch (op) {
                case EQUAL:
                    for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                        SIMD_512iTYPE vl = _mm512_load_epi64(&left[ti << 3]);
                        masks[ti] = _mm512_cmpeq_epi64_mask(vl, cr);
                    }
                    break;
                case NEQUAL:
                    for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                        SIMD_512iTYPE vl = _mm512_load_epi64(&left[ti << 3]);
                        masks[ti] = _mm512_cmpneq_epi64_mask(vl, cr);
                    }
                    break;
                case LESS:
                    for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                        SIMD_512iTYPE vl = _mm512_load_epi64(&left[ti << 3]);
                        masks[ti] = _mm512_cmplt_epi64_mask(vl, cr);
                    }
                    break;
                case LESSEQ:
                    for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                        SIMD_512iTYPE vl = _mm512_load_epi64(&left[ti << 3]);
                        masks[ti] = _mm512_cmple_epi64_mask(vl, cr);
                    }
                    break;
                case GREATEQ:
                    for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                        SIMD_512iTYPE vl = _mm512_load_epi64(&left[ti << 3]);
                        masks[ti] = _mm512_cmpge_epi64_mask(vl, cr);
                    }
                    break;
                case GREATER:
                    for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                        SIMD_512iTYPE vl = _mm512_load_epi64(&left[ti << 3]);
                        masks[ti] = _mm512_cmpgt_epi64_mask(vl, cr);
                    }
                    break;
                default:
                    break;
            }
        }
    }

   public:
    PredVEvaluator(uint8_t *_masks) : masks(_masks) {}
    void evaluate(QueryContext::OpTree *tree, data64 **bufArray) {
        Value *lvalue;
        if (tree->left->evalType == OP) {
            if (tree->left->type == DOUBLE) {
                OpVEvaluator<double> leftOp;
                lvalue = (Value *)leftOp.evaluate(tree->left, bufArray);
            } else if (tree->left->type == INT) {
                OpVEvaluator<int64_t> leftOp;
                lvalue = (Value *)leftOp.evaluate(tree->left, bufArray);
            }
        } else if (tree->left->evalType == VAR) {
            lvalue = reinterpret_cast<Value *>(bufArray[tree->left->varKey]);
        }

        Value *rvalue;
        if (tree->right->evalType == OP) {
            if (tree->right->type == DOUBLE) {
                OpVEvaluator<double> rightOp;
                rvalue = (Value *)rightOp.evaluate(tree->right, bufArray);
            } else if (tree->right->type == INT) {
                OpVEvaluator<int64_t> rightOp;
                rvalue = (Value *)rightOp.evaluate(tree->right, bufArray);
            }
            evalVar(lvalue, rvalue, tree->opType);
        } else if (tree->right->evalType == VAR) {
            rvalue = reinterpret_cast<Value *>(bufArray[tree->right->varKey]);
            evalVar(lvalue, rvalue, tree->opType);
        } else if (tree->right->evalType == CONST) {
            if (tree->right->type == DOUBLE) {
                evalConst(lvalue, (Value)tree->right->constData.d,
                          tree->opType);
            } else if (tree->right->type == INT) {
                evalConst(lvalue, (Value)tree->right->constData.i,
                          tree->opType);
            }
        }
    }

    void evaluateText(QueryContext::OpTree *tree, data64 **bufArray,
                      SIZE_TYPE **sizes) {
        // todo: add to comparison for string over 32 characters
        int lk = tree->left->varKey;
        int buf[8];
        switch (tree->opType) {
            case 0:
                if (tree->right->evalType == VAR) {
                    int rk = tree->right->varKey;
                    for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                        for (int tii = 0; tii < 8; tii++) {
                            SIMD_256iTYPE vl = _mm256_loadu_si256(
                                reinterpret_cast<SIMD_256iuTYPE *>(
                                    bufArray[lk][tii].p));
                            SIMD_256iTYPE vr = _mm256_loadu_si256(
                                reinterpret_cast<SIMD_256iuTYPE *>(
                                    bufArray[rk][tii].p));
                            uint32_t v = _mm256_cmpeq_epi8_mask(vl, vr);
                            buf[tii] = __builtin_ffs(~v | (1 << 31));
                        }

                        // SIMD_256iTYPE v = _mm256_set_epi64x(vi0, vi1, vi2,
                        // vi3);
                        SIMD_256iTYPE v = _mm256_load_epi32(buf);
                        SIMD_256iTYPE size =
                            _mm256_load_epi32(&sizes[lk][ti << 3]);
                        masks[ti] = _mm256_cmpgt_epi32_mask(v, size);
                    }
                } else {
                    SIMD_256iTYPE cr =
                        _mm256_loadu_si256(reinterpret_cast<SIMD_256iuTYPE *>(
                            tree->right->constData.p));

                    for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                        for (int tii = 0; tii < 8; tii++) {
                            SIMD_256iTYPE vl = _mm256_loadu_si256(
                                reinterpret_cast<SIMD_256iuTYPE *>(
                                    bufArray[lk][tii].p));
                            uint32_t v = _mm256_cmpeq_epi8_mask(vl, cr);
                            buf[tii] = __builtin_ffs(~v | (1 << 31));
                        }

                        // SIMD_256iTYPE v = _mm256_set_epi64x(vi0, vi1, vi2,
                        // vi3);
                        SIMD_256iTYPE v = _mm256_load_epi32(buf);
                        SIMD_256iTYPE size =
                            _mm256_load_epi32(&sizes[lk][ti << 3]);
                        masks[ti] = _mm256_cmpgt_epi32_mask(v, size);
                    }
                }
                break;
            case 12:
                if (tree->right->evalType == VAR) {
                    int rk = tree->right->varKey;
                    for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                        for (int tii = 0; tii < 8; tii++) {
                            SIMD_256iTYPE vl = _mm256_loadu_si256(
                                reinterpret_cast<SIMD_256iuTYPE *>(
                                    bufArray[lk][tii].p));
                            SIMD_256iTYPE vr = _mm256_loadu_si256(
                                reinterpret_cast<SIMD_256iuTYPE *>(
                                    bufArray[rk][tii].p));
                            uint32_t v = _mm256_cmpneq_epi8_mask(vl, vr);
                            buf[tii] = __builtin_ffs(~v | (1 << 31));
                        }

                        // SIMD_256iTYPE v = _mm256_set_epi64x(vi0, vi1, vi2,
                        // vi3);
                        SIMD_256iTYPE v = _mm256_load_epi32(buf);
                        SIMD_256iTYPE size =
                            _mm256_load_epi32(&sizes[lk][ti << 3]);
                        masks[ti] = _mm256_cmpgt_epi32_mask(v, size);
                    }
                } else {
                    SIMD_256iTYPE cr =
                        _mm256_loadu_si256(reinterpret_cast<SIMD_256iuTYPE *>(
                            tree->right->constData.p));
                    for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                        for (int tii = 0; tii < 8; tii++) {
                            SIMD_256iTYPE vl = _mm256_loadu_si256(
                                reinterpret_cast<SIMD_256iuTYPE *>(
                                    bufArray[lk][tii].p));
                            uint32_t v = _mm256_cmpneq_epi8_mask(vl, cr);
                            buf[tii] = __builtin_ffs(~v | (1 << 31));
                        }

                        // SIMD_256iTYPE v = _mm256_set_epi64x(vi0, vi1, vi2,
                        // vi3);
                        SIMD_256iTYPE v = _mm256_load_epi32(buf);
                        SIMD_256iTYPE size =
                            _mm256_load_epi32(&sizes[lk][ti << 3]);
                        masks[ti] = _mm256_cmpgt_epi32_mask(v, size);
                    }
                }
                break;
            case 32:
                // todo: add regex
            default:
                break;
        }
    }
};
}  // namespace vlex
