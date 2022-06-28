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
            for (int i = 0; i < VECEX_BYTE; i += 4) {
                SIMD_256dTYPE vl = _mm256_load_pd(&lvalue[i]);
                SIMD_256dTYPE vr = _mm256_load_pd(&rvalue[i]);
                SIMD_256dTYPE v = _mm256_add_pd(vl, vr);
                _mm256_store_pd(&output[i], v);
            }
        } else if constexpr (std::is_same_v<Value, int64_t>) {
            for (int i = 0; i < VECEX_BYTE; i += 4) {
                SIMD_256iTYPE vl =
                    _mm256_load_si256((SIMD_256iTYPE *)&lvalue[i]);
                SIMD_256iTYPE vr =
                    _mm256_load_si256((SIMD_256iTYPE *)&rvalue[i]);
                SIMD_256iTYPE v = _mm256_add_epi64(vl, vr);
                _mm256_store_si256((SIMD_256iTYPE *)&output[i], v);
            }
        }
    }

    void evalSub(Value *lvalue, Value *rvalue, Value *output) {
        if constexpr (std::is_same_v<Value, double>) {
            for (int i = 0; i < VECEX_BYTE; i += 4) {
                SIMD_256dTYPE vl = _mm256_load_pd(&lvalue[i]);
                SIMD_256dTYPE vr = _mm256_load_pd(&rvalue[i]);
                SIMD_256dTYPE v = _mm256_sub_pd(vl, vr);
                _mm256_store_pd(&output[i], v);
            }
        } else if constexpr (std::is_same_v<Value, int64_t>) {
            for (int i = 0; i < VECEX_BYTE; i += 4) {
                SIMD_256iTYPE vl =
                    _mm256_load_si256((SIMD_256iTYPE *)&lvalue[i]);
                SIMD_256iTYPE vr =
                    _mm256_load_si256((SIMD_256iTYPE *)&rvalue[i]);
                SIMD_256iTYPE v = _mm256_sub_epi64(vl, vr);
                _mm256_store_si256((SIMD_256iTYPE *)&output[i], v);
            }
        }
    }

    void evalMul(Value *lvalue, Value *rvalue, Value *output) {
        if constexpr (std::is_same_v<Value, double>) {
            for (int i = 0; i < VECEX_BYTE; i += 4) {
                SIMD_256dTYPE vl = _mm256_load_pd(&lvalue[i]);
                SIMD_256dTYPE vr = _mm256_load_pd(&rvalue[i]);
                SIMD_256dTYPE v = _mm256_mul_pd(vl, vr);
                _mm256_store_pd(&output[i], v);
            }
        } else if constexpr (std::is_same_v<Value, int64_t>) {
            // for (int i = 0; i < VECEX_BYTE; i += 4) {
            //     SIMD_256iTYPE vl = _mm256_load_epi64(&lvalue[i]);
            //     SIMD_256iTYPE vr = _mm256_load_epi64(&rvalue[i]);
            //     SIMD_256iTYPE v = _mm256_mul_epi64(vl, vr);
            //     _mm256_store_epi64(&output[i], v);
            // }
            std::cout << "error: No instruction." << std::endl;
        }
    }

    void evalDiv(Value *lvalue, Value *rvalue, Value *output) {
        if constexpr (std::is_same_v<Value, double>) {
            for (int i = 0; i < VECEX_BYTE; i += 4) {
                SIMD_256dTYPE vl = _mm256_load_pd(&lvalue[i]);
                SIMD_256dTYPE vr = _mm256_load_pd(&rvalue[i]);
                SIMD_256dTYPE v = _mm256_div_pd(vl, vr);
                _mm256_store_pd(&output[i], v);
            }
        } else if constexpr (std::is_same_v<Value, int64_t>) {
            // for (int i = 0; i < VECEX_BYTE; i += 4) {
            //     SIMD_256iTYPE vl = _mm256_load_epi64(&lvalue[i]);
            //     SIMD_256iTYPE vr = _mm256_load_epi64(&rvalue[i]);
            //     SIMD_256iTYPE v = _mm256_div_epi64(vl, vr);
            //     _mm256_store_epi64(&output[i], v);
            // }
            std::cout << "error: No instruction." << std::endl;
        }
    }

    void evalAddConst(Value *lvalue, Value rvalue, Value *output) {
        if constexpr (std::is_same_v<Value, double>) {
            SIMD_256dTYPE cr = _mm256_set1_pd(rvalue);
            for (int i = 0; i < VECEX_BYTE; i += 4) {
                SIMD_256dTYPE vl = _mm256_load_pd(&lvalue[i]);
                SIMD_256dTYPE v = _mm256_add_pd(vl, cr);
                _mm256_store_pd(&output[i], v);
            }
        } else if constexpr (std::is_same_v<Value, int64_t>) {
            SIMD_256iTYPE cr = _mm256_set1_epi64x(rvalue);
            for (int i = 0; i < VECEX_BYTE; i += 4) {
                SIMD_256iTYPE vl =
                    _mm256_load_si256((SIMD_256iTYPE *)&lvalue[i]);
                SIMD_256iTYPE v = _mm256_add_epi64(vl, cr);
                _mm256_store_si256((SIMD_256iTYPE *)&output[i], v);
            }
        }
    }

    void evalSubConst(Value *lvalue, Value rvalue, Value *output) {
        if constexpr (std::is_same_v<Value, double>) {
            SIMD_256dTYPE cr = _mm256_set1_pd(rvalue);
            for (int i = 0; i < VECEX_BYTE; i += 4) {
                SIMD_256dTYPE vl = _mm256_load_pd(&lvalue[i]);
                SIMD_256dTYPE v = _mm256_sub_pd(vl, cr);
                _mm256_store_pd(&output[i], v);
            }
        } else if constexpr (std::is_same_v<Value, int64_t>) {
            SIMD_256iTYPE cr = _mm256_set1_epi64x(rvalue);
            for (int i = 0; i < VECEX_BYTE; i += 4) {
                SIMD_256iTYPE vl =
                    _mm256_load_si256((SIMD_256iTYPE *)&lvalue[i]);
                SIMD_256iTYPE v = _mm256_sub_epi64(vl, cr);
                _mm256_store_si256((SIMD_256iTYPE *)&output[i], v);
            }
        }
    }

    void evalMulConst(Value *lvalue, Value rvalue, Value *output) {
        if constexpr (std::is_same_v<Value, double>) {
            SIMD_256dTYPE cr = _mm256_set1_pd(rvalue);
            for (int i = 0; i < VECEX_BYTE; i += 4) {
                SIMD_256dTYPE vl = _mm256_load_pd(&lvalue[i]);
                SIMD_256dTYPE v = _mm256_mul_pd(vl, cr);
                _mm256_store_pd(&output[i], v);
            }
        } else if constexpr (std::is_same_v<Value, int64_t>) {
            // SIMD_256iTYPE cr = _mm256_set1_epi64x(rvalue);
            // for (int i = 0; i < VECEX_BYTE; i += 4) {
            //     SIMD_256iTYPE vl = _mm256_load_epi64(&lvalue[i]);
            //     SIMD_256iTYPE v = _mm256_mul_epi64(vl, cr);
            //     _mm256_store_epi64(&output[i], v);
            // }
            std::cout << "error: No instruction." << std::endl;
        }
    }

    void evalDivConst(Value *lvalue, Value rvalue, Value *output) {
        if constexpr (std::is_same_v<Value, double>) {
            SIMD_256dTYPE cr = _mm256_set1_pd(rvalue);
            for (int i = 0; i < VECEX_BYTE; i += 4) {
                SIMD_256dTYPE vl = _mm256_load_pd(&lvalue[i]);
                SIMD_256dTYPE v = _mm256_div_pd(vl, cr);
                _mm256_store_pd(&output[i], v);
            }
        } else if constexpr (std::is_same_v<Value, int64_t>) {
            // for (int i = 0; i < VECEX_BYTE; i += 4) {
            //     SIMD_256iTYPE vl = _mm256_load_epi64(&lvalue[i]);
            //     SIMD_256iTYPE vr = _mm256_load_epi64(&rvalue[i]);
            //     SIMD_256iTYPE v = _mm256_div_epi64(vl, vr);
            //     _mm256_store_epi64(&output[i], v);
            // }
            std::cout << "error: No instruction." << std::endl;
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
    SIMD_256iTYPE *masks;

    void evalVar(Value *left, Value *right, OpType op) {
        if constexpr (std::is_same_v<Value, double>) {
            switch (op) {
                case EQUAL:
                    for (int ti = 0; ti < VECEX_BYTE4; ti++) {
                        SIMD_256dTYPE vl = _mm256_load_pd(&left[ti << 2]);
                        SIMD_256dTYPE vr = _mm256_load_pd(&right[ti << 2]);
                        SIMD_256TYPE v =
                            (SIMD_256TYPE)_mm256_cmp_pd(vl, vr, _CMP_EQ_OQ);
                        _mm256_store_si256(&masks[ti], (SIMD_256iTYPE)v);
                    }
                    break;
                case NEQUAL:
                    for (int ti = 0; ti < VECEX_BYTE4; ti++) {
                        SIMD_256dTYPE vl = _mm256_load_pd(&left[ti << 2]);
                        SIMD_256dTYPE vr = _mm256_load_pd(&right[ti << 2]);
                        SIMD_256TYPE v =
                            (SIMD_256TYPE)_mm256_cmp_pd(vl, vr, _CMP_NEQ_OQ);
                        _mm256_store_si256(&masks[ti], (SIMD_256iTYPE)v);
                    }
                    break;
                case LESS:
                    for (int ti = 0; ti < VECEX_BYTE4; ti++) {
                        SIMD_256dTYPE vl = _mm256_load_pd(&left[ti << 2]);
                        SIMD_256dTYPE vr = _mm256_load_pd(&right[ti << 2]);
                        SIMD_256TYPE v =
                            (SIMD_256TYPE)_mm256_cmp_pd(vl, vr, _CMP_LT_OQ);
                        _mm256_store_si256(&masks[ti], (SIMD_256iTYPE)v);
                    }
                    break;
                case LESSEQ:
                    for (int ti = 0; ti < VECEX_BYTE4; ti++) {
                        SIMD_256dTYPE vl = _mm256_load_pd(&left[ti << 2]);
                        SIMD_256dTYPE vr = _mm256_load_pd(&right[ti << 2]);
                        SIMD_256TYPE v =
                            (SIMD_256TYPE)_mm256_cmp_pd(vl, vr, _CMP_LE_OQ);
                        _mm256_store_si256(&masks[ti], (SIMD_256iTYPE)v);
                    }
                    break;
                case GREATEQ:
                    for (int ti = 0; ti < VECEX_BYTE4; ti++) {
                        SIMD_256dTYPE vl = _mm256_load_pd(&left[ti << 2]);
                        SIMD_256dTYPE vr = _mm256_load_pd(&right[ti << 2]);
                        SIMD_256TYPE v =
                            (SIMD_256TYPE)_mm256_cmp_pd(vl, vr, _CMP_GE_OQ);
                        _mm256_store_si256(&masks[ti], (SIMD_256iTYPE)v);
                    }
                    break;
                case GREATER:
                    for (int ti = 0; ti < VECEX_BYTE4; ti++) {
                        SIMD_256dTYPE vl = _mm256_load_pd(&left[ti << 2]);
                        SIMD_256dTYPE vr = _mm256_load_pd(&right[ti << 2]);
                        SIMD_256TYPE v =
                            (SIMD_256TYPE)_mm256_cmp_pd(vl, vr, _CMP_GT_OQ);
                        _mm256_store_si256(&masks[ti], (SIMD_256iTYPE)v);
                    }
                    break;
            }
        } else if constexpr (std::is_same_v<Value, int64_t>) {
            const SIMD_256iTYPE ones = _mm256_set1_epi64x(MAX_64BYTE);
            switch (op) {
                case EQUAL:
                    for (int ti = 0; ti < VECEX_BYTE4; ti++) {
                        SIMD_256iTYPE vl =
                            _mm256_load_si256((SIMD_256iTYPE *)&left[ti << 2]);
                        SIMD_256iTYPE vr =
                            _mm256_load_si256((SIMD_256iTYPE *)&right[ti << 2]);
                        SIMD_256iTYPE v = _mm256_cmpeq_epi64(vl, vr);
                        _mm256_store_si256(&masks[ti], v);
                    }
                    break;
                case NEQUAL:
                    for (int ti = 0; ti < VECEX_BYTE4; ti++) {
                        SIMD_256iTYPE vl =
                            _mm256_load_si256((SIMD_256iTYPE *)&left[ti << 2]);
                        SIMD_256iTYPE vr =
                            _mm256_load_si256((SIMD_256iTYPE *)&right[ti << 2]);
                        SIMD_256iTYPE r1 = _mm256_cmpeq_epi64(vl, vr);
                        SIMD_256iTYPE v = _mm256_xor_si256(r1, ones);
                        _mm256_store_si256(&masks[ti], v);
                    }
                    break;
                case LESS:
                    for (int ti = 0; ti < VECEX_BYTE4; ti++) {
                        SIMD_256iTYPE vl =
                            _mm256_load_si256((SIMD_256iTYPE *)&left[ti << 2]);
                        SIMD_256iTYPE vr =
                            _mm256_load_si256((SIMD_256iTYPE *)&right[ti << 2]);
                        SIMD_256iTYPE r1 = _mm256_cmpgt_epi64(vl, vr);
                        SIMD_256iTYPE r2 = _mm256_cmpeq_epi64(vl, vr);
                        SIMD_256iTYPE v =
                            _mm256_xor_si256(_mm256_or_si256(r1, r2), ones);
                        _mm256_store_si256(&masks[ti], v);
                    }
                    break;
                case LESSEQ:
                    for (int ti = 0; ti < VECEX_BYTE4; ti++) {
                        SIMD_256iTYPE vl =
                            _mm256_load_si256((SIMD_256iTYPE *)&left[ti << 2]);
                        SIMD_256iTYPE vr =
                            _mm256_load_si256((SIMD_256iTYPE *)&right[ti << 2]);
                        SIMD_256iTYPE r1 = _mm256_cmpgt_epi64(vl, vr);
                        SIMD_256iTYPE v = _mm256_xor_si256(r1, ones);
                        _mm256_store_si256(&masks[ti], v);
                    }
                    break;
                case GREATEQ:
                    for (int ti = 0; ti < VECEX_BYTE4; ti++) {
                        SIMD_256iTYPE vl =
                            _mm256_load_si256((SIMD_256iTYPE *)&left[ti << 2]);
                        SIMD_256iTYPE vr =
                            _mm256_load_si256((SIMD_256iTYPE *)&right[ti << 2]);
                        SIMD_256iTYPE r1 = _mm256_cmpgt_epi64(vl, vr);
                        SIMD_256iTYPE r2 = _mm256_cmpeq_epi64(vl, vr);
                        SIMD_256iTYPE v = _mm256_or_si256(r1, r2);
                        _mm256_store_si256(&masks[ti], v);
                    }
                    break;
                case GREATER:
                    for (int ti = 0; ti < VECEX_BYTE4; ti++) {
                        SIMD_256iTYPE vl =
                            _mm256_load_si256((SIMD_256iTYPE *)&left[ti << 2]);
                        SIMD_256iTYPE vr =
                            _mm256_load_si256((SIMD_256iTYPE *)&right[ti << 2]);
                        SIMD_256iTYPE v = _mm256_cmpgt_epi64(vl, vr);
                        _mm256_store_si256(&masks[ti], v);
                    }
                    break;
                default:
                    break;
            }
        }
    }

    void evalConst(Value *left, Value right, OpType op) {
        if constexpr (std::is_same_v<Value, double>) {
            SIMD_256dTYPE cr = _mm256_set1_pd(right);
            switch (op) {
                case EQUAL:
                    for (int ti = 0; ti < VECEX_BYTE4; ti++) {
                        SIMD_256dTYPE vl = _mm256_load_pd(&left[ti << 2]);
                        SIMD_256TYPE v =
                            (SIMD_256TYPE)_mm256_cmp_pd(vl, cr, _CMP_EQ_OQ);
                        _mm256_store_si256(&masks[ti], (SIMD_256iTYPE)v);
                    }
                    break;
                case NEQUAL:
                    for (int ti = 0; ti < VECEX_BYTE4; ti++) {
                        SIMD_256dTYPE vl = _mm256_load_pd(&left[ti << 2]);
                        SIMD_256TYPE v =
                            (SIMD_256TYPE)_mm256_cmp_pd(vl, cr, _CMP_NEQ_OQ);
                        _mm256_store_si256(&masks[ti], (SIMD_256iTYPE)v);
                    }
                    break;
                case LESS:
                    for (int ti = 0; ti < VECEX_BYTE4; ti++) {
                        SIMD_256dTYPE vl = _mm256_load_pd(&left[ti << 2]);
                        SIMD_256TYPE v =
                            (SIMD_256TYPE)_mm256_cmp_pd(vl, cr, _CMP_LT_OQ);
                        _mm256_store_si256(&masks[ti], (SIMD_256iTYPE)v);
                    }
                    break;
                case LESSEQ:
                    for (int ti = 0; ti < VECEX_BYTE4; ti++) {
                        SIMD_256dTYPE vl = _mm256_load_pd(&left[ti << 2]);
                        SIMD_256TYPE v =
                            (SIMD_256TYPE)_mm256_cmp_pd(vl, cr, _CMP_LE_OQ);
                        _mm256_store_si256(&masks[ti], (SIMD_256iTYPE)v);
                    }
                    break;
                case GREATEQ:
                    for (int ti = 0; ti < VECEX_BYTE4; ti++) {
                        SIMD_256dTYPE vl = _mm256_load_pd(&left[ti << 2]);
                        SIMD_256TYPE v =
                            (SIMD_256TYPE)_mm256_cmp_pd(vl, cr, _CMP_GE_OQ);
                        _mm256_store_si256(&masks[ti], (SIMD_256iTYPE)v);
                    }
                    break;
                case GREATER:
                    for (int ti = 0; ti < VECEX_BYTE4; ti++) {
                        SIMD_256dTYPE vl = _mm256_load_pd(&left[ti << 2]);
                        SIMD_256TYPE v =
                            (SIMD_256TYPE)_mm256_cmp_pd(vl, cr, _CMP_GT_OQ);
                        _mm256_store_si256(&masks[ti], (SIMD_256iTYPE)v);
                    }
                    break;
                default:
                    break;
            }
        } else if constexpr (std::is_same_v<Value, int64_t>) {
            SIMD_256iTYPE cr = _mm256_set1_epi64x(right);
            const SIMD_256iTYPE ones = _mm256_set1_epi64x(MAX_64BYTE);
            switch (op) {
                case EQUAL:
                    for (int ti = 0; ti < VECEX_BYTE4; ti++) {
                        SIMD_256iTYPE vl =
                            _mm256_load_si256((SIMD_256iTYPE *)&left[ti << 2]);
                        SIMD_256iTYPE v = _mm256_cmpeq_epi64(vl, cr);
                        _mm256_store_si256(&masks[ti], v);
                    }
                    break;
                case NEQUAL:
                    for (int ti = 0; ti < VECEX_BYTE4; ti++) {
                        SIMD_256iTYPE vl =
                            _mm256_load_si256((SIMD_256iTYPE *)&left[ti << 2]);
                        SIMD_256iTYPE r1 = _mm256_cmpeq_epi64(vl, cr);
                        SIMD_256iTYPE v = _mm256_xor_si256(r1, ones);
                        _mm256_store_si256(&masks[ti], v);
                    }
                    break;
                case LESS:
                    for (int ti = 0; ti < VECEX_BYTE4; ti++) {
                        SIMD_256iTYPE vl =
                            _mm256_load_si256((SIMD_256iTYPE *)&left[ti << 2]);
                        SIMD_256iTYPE r1 = _mm256_cmpgt_epi64(vl, cr);
                        SIMD_256iTYPE r2 = _mm256_cmpeq_epi64(vl, cr);
                        SIMD_256iTYPE v =
                            _mm256_xor_si256(_mm256_or_si256(r1, r2), ones);
                        _mm256_store_si256(&masks[ti], v);
                    }
                    break;
                case LESSEQ:
                    for (int ti = 0; ti < VECEX_BYTE4; ti++) {
                        SIMD_256iTYPE vl =
                            _mm256_load_si256((SIMD_256iTYPE *)&left[ti << 2]);
                        SIMD_256iTYPE r1 = _mm256_cmpgt_epi64(vl, cr);
                        SIMD_256iTYPE v = _mm256_xor_si256(r1, ones);
                        _mm256_store_si256(&masks[ti], v);
                    }
                    break;
                case GREATEQ:
                    for (int ti = 0; ti < VECEX_BYTE4; ti++) {
                        SIMD_256iTYPE vl =
                            _mm256_load_si256((SIMD_256iTYPE *)&left[ti << 2]);
                        SIMD_256iTYPE r1 = _mm256_cmpgt_epi64(vl, cr);
                        SIMD_256iTYPE r2 = _mm256_cmpeq_epi64(vl, cr);
                        SIMD_256iTYPE v = _mm256_or_si256(r1, r2);
                        _mm256_store_si256(&masks[ti], v);
                    }
                    break;
                case GREATER:
                    for (int ti = 0; ti < VECEX_BYTE4; ti++) {
                        SIMD_256iTYPE vl =
                            _mm256_load_si256((SIMD_256iTYPE *)&left[ti << 2]);
                        SIMD_256iTYPE v = _mm256_cmpgt_epi64(vl, cr);
                        _mm256_store_si256(&masks[ti], v);
                    }
                    break;
                default:
                    break;
            }
        }
    }

   public:
    PredVEvaluator(SIMD_256iTYPE *_masks) : masks(_masks) {}
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

    void evaluateText(QueryContext::OpTree *tree, DATA_TYPE *data,
                      SIZE_TYPE **indexes, SIZE_TYPE **sizes) {
        // todo: add to comparison for string over 32 characters
        int lk = tree->left->varKey;
        switch (tree->opType) {
            case 0:
                if (tree->right->evalType == VAR) {
                    int rk = tree->right->varKey;
                    for (int ti = 0; ti < VECEX_BYTE; ti += 4) {
                        SIMD_256iTYPE vl0 = _mm256_loadu_si256(
                            reinterpret_cast<SIMD_256iuTYPE *>(
                                &data[indexes[lk][ti]]));
                        SIMD_256iTYPE vr0 = _mm256_loadu_si256(
                            reinterpret_cast<SIMD_256iuTYPE *>(
                                &data[indexes[rk][ti]]));
                        SIMD_256iTYPE v0 = _mm256_cmpeq_epi8(vl0, vr0);
                        int64_t vi0 = (int64_t)__builtin_ffs(
                            ~_mm256_movemask_epi8(v0) | (1 << 31));

                        SIMD_256iTYPE vl1 = _mm256_loadu_si256(
                            reinterpret_cast<SIMD_256iuTYPE *>(
                                &data[indexes[lk][ti + 1]]));
                        SIMD_256iTYPE vr1 = _mm256_loadu_si256(
                            reinterpret_cast<SIMD_256iuTYPE *>(
                                &data[indexes[rk][ti + 1]]));
                        SIMD_256iTYPE v1 = _mm256_cmpeq_epi8(vl1, vr1);
                        int64_t vi1 = (int64_t)__builtin_ffs(
                            ~_mm256_movemask_epi8(v1) | (1 << 31));

                        SIMD_256iTYPE vl2 = _mm256_loadu_si256(
                            reinterpret_cast<SIMD_256iuTYPE *>(
                                &data[indexes[lk][ti + 2]]));
                        SIMD_256iTYPE vr2 = _mm256_loadu_si256(
                            reinterpret_cast<SIMD_256iuTYPE *>(
                                &data[indexes[rk][ti + 2]]));
                        SIMD_256iTYPE v2 = _mm256_cmpeq_epi8(vl2, vr2);
                        int64_t vi2 = (int64_t)__builtin_ffs(
                            ~_mm256_movemask_epi8(v2) | (1 << 31));

                        SIMD_256iTYPE vl3 = _mm256_loadu_si256(
                            reinterpret_cast<SIMD_256iuTYPE *>(
                                &data[indexes[lk][ti + 3]]));
                        SIMD_256iTYPE vr3 = _mm256_loadu_si256(
                            reinterpret_cast<SIMD_256iuTYPE *>(
                                &data[indexes[rk][ti + 3]]));
                        SIMD_256iTYPE v3 = _mm256_cmpeq_epi8(vl3, vr3);
                        int64_t vi3 = (int64_t)__builtin_ffs(
                            ~_mm256_movemask_epi8(v3) | (1 << 31));

                        SIMD_256iTYPE v = _mm256_set_epi64x(vi0, vi1, vi2, vi3);

                        SIMD_128iTYPE size = _mm_load_si128(
                            reinterpret_cast<SIMD_128iTYPE *>(&sizes[lk][ti]));
                        SIMD_256iTYPE size64 = _mm256_cvtepi32_epi64(size);
                        SIMD_256iTYPE m = _mm256_cmpgt_epi64(v, size64);
                        _mm256_store_si256(&masks[ti >> 2], m);
                    }
                } else {
                    SIMD_256iTYPE cr =
                        _mm256_loadu_si256(reinterpret_cast<SIMD_256iuTYPE *>(
                            tree->right->constData.p));
                    for (int ti = 0; ti < VECEX_BYTE; ti += 4) {
                        SIMD_256iTYPE vl0 = _mm256_loadu_si256(
                            reinterpret_cast<SIMD_256iuTYPE *>(
                                &data[indexes[lk][ti]]));
                        SIMD_256iTYPE v0 = _mm256_cmpeq_epi8(vl0, cr);
                        int64_t vi0 = (int64_t)__builtin_ffs(
                            ~_mm256_movemask_epi8(v0) | (1 << 31));

                        SIMD_256iTYPE vl1 = _mm256_loadu_si256(
                            reinterpret_cast<SIMD_256iuTYPE *>(
                                &data[indexes[lk][ti + 1]]));
                        SIMD_256iTYPE v1 = _mm256_cmpeq_epi8(vl1, cr);
                        int64_t vi1 = (int64_t)__builtin_ffs(
                            ~_mm256_movemask_epi8(v1) | (1 << 31));

                        SIMD_256iTYPE vl2 = _mm256_loadu_si256(
                            reinterpret_cast<SIMD_256iuTYPE *>(
                                &data[indexes[lk][ti + 2]]));
                        SIMD_256iTYPE v2 = _mm256_cmpeq_epi8(vl2, cr);
                        int64_t vi2 = (int64_t)__builtin_ffs(
                            ~_mm256_movemask_epi8(v2) | (1 << 31));

                        SIMD_256iTYPE vl3 = _mm256_loadu_si256(
                            reinterpret_cast<SIMD_256iuTYPE *>(
                                &data[indexes[lk][ti + 3]]));
                        SIMD_256iTYPE v3 = _mm256_cmpeq_epi8(vl3, cr);
                        int64_t vi3 = (int64_t)__builtin_ffs(
                            ~_mm256_movemask_epi8(v3) | (1 << 31));

                        SIMD_256iTYPE v = _mm256_set_epi64x(vi0, vi1, vi2, vi3);

                        SIMD_128iTYPE size = _mm_load_si128(
                            reinterpret_cast<SIMD_128iTYPE *>(&sizes[lk][ti]));
                        SIMD_256iTYPE size64 = _mm256_cvtepi32_epi64(size);
                        SIMD_256iTYPE m = _mm256_cmpgt_epi64(v, size64);
                        _mm256_store_si256(&masks[ti >> 2], m);
                    }
                }
                break;
            case 12:
                if (tree->right->evalType == VAR) {
                    int rk = tree->right->varKey;
                    for (int ti = 0; ti < VECEX_BYTE; ti += 4) {
                        SIMD_256iTYPE vl0 = _mm256_loadu_si256(
                            reinterpret_cast<SIMD_256iuTYPE *>(
                                &data[indexes[lk][ti]]));
                        SIMD_256iTYPE vr0 = _mm256_loadu_si256(
                            reinterpret_cast<SIMD_256iuTYPE *>(
                                &data[indexes[rk][ti]]));
                        SIMD_256iTYPE v0 = _mm256_cmpeq_epi8(vl0, vr0);
                        int64_t vi0 =
                            (int64_t)__builtin_ffs(~_mm256_movemask_epi8(v0) |
                                                   (1 << 31)) -
                            1;

                        SIMD_256iTYPE vl1 = _mm256_loadu_si256(
                            reinterpret_cast<SIMD_256iuTYPE *>(
                                &data[indexes[lk][ti + 1]]));
                        SIMD_256iTYPE vr1 = _mm256_loadu_si256(
                            reinterpret_cast<SIMD_256iuTYPE *>(
                                &data[indexes[rk][ti + 1]]));
                        SIMD_256iTYPE v1 = _mm256_cmpeq_epi8(vl1, vr1);
                        int64_t vi1 =
                            (int64_t)__builtin_ffs(~_mm256_movemask_epi8(v1) |
                                                   (1 << 31)) -
                            1;

                        SIMD_256iTYPE vl2 = _mm256_loadu_si256(
                            reinterpret_cast<SIMD_256iuTYPE *>(
                                &data[indexes[lk][ti + 2]]));
                        SIMD_256iTYPE vr2 = _mm256_loadu_si256(
                            reinterpret_cast<SIMD_256iuTYPE *>(
                                &data[indexes[rk][ti + 2]]));
                        SIMD_256iTYPE v2 = _mm256_cmpeq_epi8(vl2, vr2);
                        int64_t vi2 =
                            (int64_t)__builtin_ffs(~_mm256_movemask_epi8(v2) |
                                                   (1 << 31)) -
                            1;

                        SIMD_256iTYPE vl3 = _mm256_loadu_si256(
                            reinterpret_cast<SIMD_256iuTYPE *>(
                                &data[indexes[lk][ti + 3]]));
                        SIMD_256iTYPE vr3 = _mm256_loadu_si256(
                            reinterpret_cast<SIMD_256iuTYPE *>(
                                &data[indexes[rk][ti + 3]]));
                        SIMD_256iTYPE v3 = _mm256_cmpeq_epi8(vl3, vr3);
                        int64_t vi3 =
                            (int64_t)__builtin_ffs(
                                (~_mm256_movemask_epi8(v3) | (1 << 31))) -
                            1;

                        SIMD_256iTYPE v = _mm256_set_epi64x(vi0, vi1, vi2, vi3);

                        SIMD_128iTYPE size = _mm_load_si128(
                            reinterpret_cast<SIMD_128iTYPE *>(&sizes[lk][ti]));
                        SIMD_256iTYPE size64 = _mm256_cvtepi32_epi64(size);
                        SIMD_256iTYPE m = _mm256_cmpgt_epi64(size64, v);
                        _mm256_store_si256(&masks[ti >> 2], m);
                    }
                } else {
                    SIMD_256iTYPE cr =
                        _mm256_loadu_si256(reinterpret_cast<SIMD_256iuTYPE *>(
                            tree->right->constData.p));
                    for (int ti = 0; ti < VECEX_BYTE; ti += 4) {
                        SIMD_256iTYPE vl0 = _mm256_loadu_si256(
                            reinterpret_cast<SIMD_256iuTYPE *>(
                                &data[indexes[lk][ti]]));
                        SIMD_256iTYPE v0 = _mm256_cmpeq_epi8(vl0, cr);
                        int64_t vi0 =
                            (int64_t)__builtin_ffs(~_mm256_movemask_epi8(v0) |
                                                   (1 << 31)) -
                            1;

                        SIMD_256iTYPE vl1 = _mm256_loadu_si256(
                            reinterpret_cast<SIMD_256iuTYPE *>(
                                &data[indexes[lk][ti + 1]]));
                        SIMD_256iTYPE v1 = _mm256_cmpeq_epi8(vl1, cr);
                        int64_t vi1 =
                            (int64_t)__builtin_ffs(~_mm256_movemask_epi8(v1) |
                                                   (1 << 31)) -
                            1;

                        SIMD_256iTYPE vl2 = _mm256_loadu_si256(
                            reinterpret_cast<SIMD_256iuTYPE *>(
                                &data[indexes[lk][ti + 2]]));
                        SIMD_256iTYPE v2 = _mm256_cmpeq_epi8(vl2, cr);
                        int64_t vi2 =
                            (int64_t)__builtin_ffs(~_mm256_movemask_epi8(v2) |
                                                   (1 << 31)) -
                            1;

                        SIMD_256iTYPE vl3 = _mm256_loadu_si256(
                            reinterpret_cast<SIMD_256iuTYPE *>(
                                &data[indexes[lk][ti + 3]]));
                        SIMD_256iTYPE v3 = _mm256_cmpeq_epi8(vl3, cr);
                        int64_t vi3 =
                            (int64_t)__builtin_ffs(~_mm256_movemask_epi8(v3) |
                                                   (1 << 31)) -
                            1;

                        SIMD_256iTYPE v = _mm256_set_epi64x(vi0, vi1, vi2, vi3);

                        SIMD_128iTYPE size = _mm_load_si128(
                            reinterpret_cast<SIMD_128iTYPE *>(&sizes[lk][ti]));
                        SIMD_256iTYPE size64 = _mm256_cvtepi32_epi64(size);
                        SIMD_256iTYPE m = _mm256_cmpgt_epi64(size64, v);
                        _mm256_store_si256(&masks[ti >> 2], m);
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
