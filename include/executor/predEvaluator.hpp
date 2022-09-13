#pragma once

#include "common.hpp"
#include "opEvaluator.hpp"

namespace vlex {
template <typename Value>
class PredVEvaluator {
    uint8_t *masks;

    template <OpType op>
    void vevalPred(Value *l, Value *r) {
        if constexpr (std::is_same_v<Value, double>) {
#if (defined VUNROLL)
#pragma unroll(4)
#endif
            for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                SIMD_512dTYPE vl = _mm512_load_pd(&l[ti << 3]);
                SIMD_512dTYPE vr = _mm512_load_pd(&r[ti << 3]);
                if constexpr (op == EQUAL) {
                    masks[ti] = _mm512_cmp_pd_mask(vl, vr, _CMP_EQ_OQ);
                } else if constexpr (op == NEQUAL) {
                    masks[ti] = _mm512_cmp_pd_mask(vl, vr, _CMP_NEQ_OQ);
                } else if constexpr (op == LESS) {
                    masks[ti] = _mm512_cmp_pd_mask(vl, vr, _CMP_LT_OQ);
                } else if constexpr (op == LESSEQ) {
                    masks[ti] = _mm512_cmp_pd_mask(vl, vr, _CMP_LE_OQ);
                } else if constexpr (op == GREATEQ) {
                    masks[ti] = _mm512_cmp_pd_mask(vl, vr, _CMP_GE_OQ);
                } else if constexpr (op == GREATER) {
                    masks[ti] = _mm512_cmp_pd_mask(vl, vr, _CMP_GT_OQ);
                }
            }
        } else if constexpr (std::is_same_v<Value, int64_t>) {
#if (defined VUNROLL)
#pragma unroll(4)
#endif
            for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                SIMD_512iTYPE vl = _mm512_load_epi64(&l[ti << 3]);
                SIMD_512iTYPE vr = _mm512_load_epi64(&r[ti << 3]);
                if constexpr (op == EQUAL) {
                    masks[ti] = _mm512_cmp_epi64_mask(vl, vr, _MM_CMPINT_EQ);
                } else if constexpr (op == NEQUAL) {
                    masks[ti] = _mm512_cmp_epi64_mask(vl, vr, _MM_CMPINT_NE);
                } else if constexpr (op == LESS) {
                    masks[ti] = _mm512_cmp_epi64_mask(vl, vr, _MM_CMPINT_LT);
                } else if constexpr (op == LESSEQ) {
                    masks[ti] = _mm512_cmp_epi64_mask(vl, vr, _MM_CMPINT_LE);
                } else if constexpr (op == GREATEQ) {
                    masks[ti] = _mm512_cmp_epi64_mask(vl, vr, _MM_CMPINT_GE);
                } else if constexpr (op == GREATER) {
                    masks[ti] = _mm512_cmp_epi64_mask(vl, vr, _MM_CMPINT_GT);
                }
            }
        }
    }

    template <OpType op>
    void vcevalPred(Value *l, Value r) {
        if constexpr (std::is_same_v<Value, double>) {
            SIMD_512dTYPE cr = _mm512_set1_pd(r);
#if (defined VUNROLL)
#pragma unroll(4)
#endif
            for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                SIMD_512dTYPE vl = _mm512_load_pd(&l[ti << 3]);
                if constexpr (op == EQUAL) {
                    masks[ti] = _mm512_cmp_pd_mask(vl, cr, _CMP_EQ_OQ);
                } else if constexpr (op == NEQUAL) {
                    masks[ti] = _mm512_cmp_pd_mask(vl, cr, _CMP_NEQ_OQ);
                } else if constexpr (op == LESS) {
                    masks[ti] = _mm512_cmp_pd_mask(vl, cr, _CMP_LT_OQ);
                } else if constexpr (op == LESSEQ) {
                    masks[ti] = _mm512_cmp_pd_mask(vl, cr, _CMP_LE_OQ);
                } else if constexpr (op == GREATEQ) {
                    masks[ti] = _mm512_cmp_pd_mask(vl, cr, _CMP_GE_OQ);
                } else if constexpr (op == GREATER) {
                    masks[ti] = _mm512_cmp_pd_mask(vl, cr, _CMP_GT_OQ);
                }
            }
        } else if constexpr (std::is_same_v<Value, int64_t>) {
            SIMD_512iTYPE cr = _mm512_set1_epi64(r);
#if (defined VUNROLL)
#pragma unroll(4)
#endif
            for (int ti = 0; ti < VECEX_BYTE8; ti++) {
                SIMD_512iTYPE vl = _mm512_load_epi64(&l[ti << 3]);
                if constexpr (op == EQUAL) {
                    masks[ti] = _mm512_cmp_epi64_mask(vl, cr, _MM_CMPINT_EQ);
                } else if constexpr (op == NEQUAL) {
                    masks[ti] = _mm512_cmp_epi64_mask(vl, cr, _MM_CMPINT_NE);
                } else if constexpr (op == LESS) {
                    masks[ti] = _mm512_cmp_epi64_mask(vl, cr, _MM_CMPINT_LT);
                } else if constexpr (op == LESSEQ) {
                    masks[ti] = _mm512_cmp_epi64_mask(vl, cr, _MM_CMPINT_LE);
                } else if constexpr (op == GREATEQ) {
                    masks[ti] = _mm512_cmp_epi64_mask(vl, cr, _MM_CMPINT_GE);
                } else if constexpr (op == GREATER) {
                    masks[ti] = _mm512_cmp_epi64_mask(vl, cr, _MM_CMPINT_GT);
                }
            }
        }
    }

    void evalVar(Value *left, Value *right, OpType op) {
        switch (op) {
            case EQUAL:
                vevalPred<EQUAL>(left, right);
                break;
            case NEQUAL:
                vevalPred<NEQUAL>(left, right);
                break;
            case LESS:
                vevalPred<LESS>(left, right);
                break;
            case LESSEQ:
                vevalPred<LESSEQ>(left, right);
                break;
            case GREATEQ:
                vevalPred<GREATEQ>(left, right);
                break;
            case GREATER:
                vevalPred<GREATER>(left, right);
                break;
            default:
                break;
        }
    }

    void evalConst(Value *left, Value right, OpType op) {
        switch (op) {
            case EQUAL:
                vcevalPred<EQUAL>(left, right);
                break;
            case NEQUAL:
                vcevalPred<NEQUAL>(left, right);
                break;
            case LESS:
                vcevalPred<LESS>(left, right);
                break;
            case LESSEQ:
                vcevalPred<LESSEQ>(left, right);
                break;
            case GREATEQ:
                vcevalPred<GREATEQ>(left, right);
                break;
            case GREATER:
                vcevalPred<GREATER>(left, right);
                break;
            default:
                break;
        }
    }

   public:
    PredVEvaluator(uint8_t *_masks) : masks(_masks) {}
    void evaluate(QueryContext::OpTree *tree, data64 **bufArray) {
        Value *lvalue;
        if (tree->left->evalType == OP) {
            if (tree->left->type == DOUBLE) {
                OpFullEvaluator<double> leftOp;
                lvalue = (Value *)leftOp.evaluate(tree->left, bufArray);
            } else if (tree->left->type == INT) {
                OpFullEvaluator<int64_t> leftOp;
                lvalue = (Value *)leftOp.evaluate(tree->left, bufArray);
            }
        } else if (tree->left->evalType == VAR) {
            lvalue = reinterpret_cast<Value *>(bufArray[tree->left->varKey]);
        }

        Value *rvalue;
        if (tree->right->evalType == OP) {
            if (tree->right->type == DOUBLE) {
                OpFullEvaluator<double> rightOp;
                rvalue = (Value *)rightOp.evaluate(tree->right, bufArray);
            } else if (tree->right->type == INT) {
                OpFullEvaluator<int64_t> rightOp;
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

    template <OpType op>
    void vevalText(int lk, int rk, data64 **bufArray, SIZE_TYPE **sizes) {
        int buf[8];

        for (int ti = 0; ti < VECEX_BYTE8; ti++) {
#if (defined VUNROLL)
#pragma unroll(8)
#endif
            for (int tii = 0; tii < 8; tii++) {
                SIMD_512iTYPE vl = _mm512_loadu_si512(
                    reinterpret_cast<SIMD_512iuTYPE *>(bufArray[lk][tii].p));
                SIMD_512iTYPE vr = _mm512_loadu_si512(
                    reinterpret_cast<SIMD_512iuTYPE *>(bufArray[rk][tii].p));
#if (defined __AVX512VL__) && (defined __AVX512BW__)
                uint64_t v = _mm512_cmpeq_epi8_mask(vl, vr);
#else
                // SIMD_512iTYPE vv = _mm512_cmpeq_epi8(vl, vr);
                // int v = _mm256_movemask_epi8(vv);
#endif

                if constexpr (op == EQUAL) {
                    buf[tii] = __builtin_ffsll(~v | ((uint64_t)1 << 63));
                } else if constexpr (op == NEQUAL) {
                    buf[tii] = __builtin_ffsll(v | ((uint64_t)1 << 63));
                }
            }

            SIMD_256iTYPE v =
                _mm256_load_si256(reinterpret_cast<SIMD_256iTYPE *>(buf));
            SIMD_256iTYPE size = _mm256_load_si256(
                reinterpret_cast<SIMD_256iTYPE *>(&sizes[lk][ti << 3]));
#if (defined __AVX512VL__)
            masks[ti] = _mm256_cmpgt_epi32_mask(v, size);
#else
            SIMD_256iTYPE m = _mm256_cmpgt_epi32(v, size);
            masks[ti] = (uint8_t)(_mm256_movemask_ps(
                                      reinterpret_cast<SIMD_256TYPE>(m)) &&
                                  0xff);
#endif
        }
    }

    template <OpType op>
    void vcevalText(int lk, DATA_TYPE *p, data64 **bufArray,
                    SIZE_TYPE **sizes) {
        int buf[8];
        SIMD_512iTYPE cr =
            _mm512_loadu_si512(reinterpret_cast<SIMD_512iuTYPE *>(p));

        for (int ti = 0; ti < VECEX_BYTE8; ti++) {
#if (defined VUNROLL)
#pragma unroll(8)
#endif
            for (int tii = 0; tii < 8; tii++) {
                SIMD_512iTYPE vl = _mm512_loadu_si512(
                    reinterpret_cast<SIMD_512iuTYPE *>(bufArray[lk][tii].p));
#if (defined __AVX512VL__) && (defined __AVX512BW__)
                uint64_t v = _mm512_cmpeq_epi8_mask(vl, cr);
#else
                // SIMD_256iTYPE vv = _mm256_cmpeq_epi8(vl, cr);
                // int v = _mm256_movemask_epi8(vv);
#endif

                if constexpr (op == EQUAL) {
                    buf[tii] = __builtin_ffsll(~v | ((uint64_t)1 << 63));
                } else if constexpr (op == NEQUAL) {
                    buf[tii] = __builtin_ffsll(v | ((uint64_t)1 << 63));
                }
            }

            SIMD_256iTYPE v =
                _mm256_load_si256(reinterpret_cast<SIMD_256iTYPE *>(buf));
            SIMD_256iTYPE size = _mm256_load_si256(
                reinterpret_cast<SIMD_256iTYPE *>(&sizes[lk][ti << 3]));
#if (defined __AVX512VL__)
            masks[ti] = _mm256_cmpgt_epi32_mask(v, size);
#else
            SIMD_256iTYPE m = _mm256_cmpgt_epi32(v, size);
            masks[ti] = (uint8_t)(_mm256_movemask_ps(
                                      reinterpret_cast<SIMD_256TYPE>(m)) &
                                  0xff);
#endif
        }
    }

    void evaluateText(QueryContext::OpTree *tree, data64 **bufArray,
                      SIZE_TYPE **sizes) {
        // todo: add to comparison for string over 32 characters
        int lk = tree->left->varKey;
        switch (tree->opType) {
            case 0:
                if (tree->right->evalType == VAR) {
                    vevalText<EQUAL>(lk, tree->right->varKey, bufArray, sizes);
                } else {
                    vcevalText<EQUAL>(lk, tree->right->constData.p, bufArray,
                                      sizes);
                }
                break;
            case 12:
                if (tree->right->evalType == VAR) {
                    vevalText<NEQUAL>(lk, tree->right->varKey, bufArray, sizes);
                } else {
                    vcevalText<NEQUAL>(lk, tree->right->constData.p, bufArray,
                                       sizes);
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
