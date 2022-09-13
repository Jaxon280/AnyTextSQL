#pragma once

#include "common.hpp"
#include "opEvaluator.hpp"

namespace vlex {
template <typename Value>
class PredEvaluator {
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
    PredEvaluator(uint8_t *_masks) : masks(_masks) {}
    void evaluate(QueryContext::OpTree *tree, data64 **bufArray) {
        Value *lvalue;
        if (tree->left->evalType == OP) {
            // if (tree->left->type == DOUBLE) {
            //     OpFullEvaluator<double> leftOp;
            //     lvalue = (Value *)leftOp.evaluate(tree->left, bufArray);
            // } else if (tree->left->type == INT) {
            //     OpFullEvaluator<int64_t> leftOp;
            //     lvalue = (Value *)leftOp.evaluate(tree->left, bufArray);
            // }
        } else if (tree->left->evalType == VAR) {
            lvalue = reinterpret_cast<Value *>(bufArray[tree->left->varKey]);
        }

        Value *rvalue;
        if (tree->right->evalType == OP) {
            // if (tree->right->type == DOUBLE) {
            //     OpFullEvaluator<double> rightOp;
            //     rvalue = (Value *)rightOp.evaluate(tree->right, bufArray);
            // } else if (tree->right->type == INT) {
            //     OpFullEvaluator<int64_t> rightOp;
            //     rvalue = (Value *)rightOp.evaluate(tree->right, bufArray);
            // }
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
