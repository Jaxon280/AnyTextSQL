#pragma once

#include "common.hpp"
#include "parser/query.hpp"

namespace vlex {

template <typename Value>
class OpFullEvaluator {
   public:
    OpFullEvaluator() {
        data = new (std::align_val_t{VECEX_SIZE}) Value[VECEX_BYTE];
    }
    ~OpFullEvaluator() { delete data; }

    Value *evaluate(OpTree *tree, data64 **bufArray) {
        Value *lvalue;
        Value *rvalue;

        if (tree->left->evalType == CONST) {
            if (tree->right->evalType == OP) {
                if (tree->right->type == DOUBLE) {
                    OpFullEvaluator<double> ope2;
                    rvalue = (Value *)ope2.evaluate(tree->right, bufArray);
                } else if (tree->right->type == INT) {
                    OpFullEvaluator<int64_t> ope2;
                    rvalue = (Value *)ope2.evaluate(tree->right, bufArray);
                }
            } else if (tree->right->evalType == VAR) {
                rvalue =
                    reinterpret_cast<Value *>(bufArray[tree->left->varKey]);
            }

            if (tree->type == DOUBLE) {
                evalVCFull(rvalue, tree->left->constData.d, data, tree->opType);
            } else if (tree->type == INT) {
                evalVCFull(rvalue, tree->left->constData.i, data, tree->opType);
            }
        } else {
            if (tree->left->evalType == OP) {
                if (tree->left->type == DOUBLE) {
                    OpFullEvaluator<double> ope1;
                    lvalue = (Value *)ope1.evaluate(tree->left, bufArray);
                } else if (tree->left->type == INT) {
                    OpFullEvaluator<int64_t> ope1;
                    lvalue = (Value *)ope1.evaluate(tree->left, bufArray);
                }
            } else if (tree->left->evalType == VAR) {
                lvalue =
                    reinterpret_cast<Value *>(bufArray[tree->left->varKey]);
            }

            if (tree->right->evalType != CONST) {
                if (tree->right->evalType == OP) {
                    if (tree->right->type == DOUBLE) {
                        OpFullEvaluator<double> ope2;
                        rvalue = (Value *)ope2.evaluate(tree->right, bufArray);
                    } else if (tree->right->type == INT) {
                        OpFullEvaluator<int64_t> ope2;
                        rvalue = (Value *)ope2.evaluate(tree->right, bufArray);
                    }
                } else if (tree->right->evalType == VAR) {
                    rvalue =
                        reinterpret_cast<Value *>(bufArray[tree->left->varKey]);
                }
                evalVVFull(lvalue, rvalue, data, tree->opType);
            } else {
                if (tree->type == DOUBLE) {
                    evalVCFull(lvalue, tree->right->constData.d, data,
                               tree->opType);
                } else if (tree->type == INT) {
                    evalVCFull(lvalue, tree->right->constData.i, data,
                               tree->opType);
                }
            }
        }

        return data;
    }

   private:
    Value *data;

    template <OpType op>
    void vevalOp(Value *l, Value *r, Value *o) {
        if constexpr (std::is_same_v<Value, double>) {
            SIMD_512dTYPE vl, vr, v;
#if (defined VUNROLL)
#pragma unroll(4)
#endif
            for (int i = 0; i < VECEX_BYTE; i += 8) {
                vl = _mm512_load_pd(&l[i]);
                vr = _mm512_load_pd(&r[i]);
                if constexpr (op == ADD) {
                    v = _mm512_add_pd(vl, vr);
                } else if constexpr (op == SUB) {
                    v = _mm512_sub_pd(vl, vr);
                } else if constexpr (op == MUL) {
                    v = _mm512_mul_pd(vl, vr);
                } else if constexpr (op == DIV) {
                    v = _mm512_div_pd(vl, vr);
                }
                _mm512_store_pd(&o[i], v);
            }
        } else if constexpr (std::is_same_v<Value, int64_t>) {
            SIMD_512iTYPE vl, vr, v;
#if (defined VUNROLL)
#pragma unroll(4)
#endif
            for (int i = 0; i < VECEX_BYTE; i += 8) {
                vl = _mm512_load_epi64(&l[i]);
                vr = _mm512_load_epi64(&r[i]);
                if constexpr (op == ADD) {
                    v = _mm512_add_epi64(vl, vr);
                } else if constexpr (op == SUB) {
                    v = _mm512_sub_epi64(vl, vr);
                } else if constexpr (op == MUL) {
#if (defined __AVX512DQ__)
                    v = _mm512_mullo_epi64(vl, vr);
#endif
                } else if constexpr (op == DIV) {
                    // v = _mm512_div_pd(vl, vr);
                }
                _mm512_store_epi64(&o[i], v);
            }
        }
    }

    template <OpType op>
    void vcevalOp(Value *l, Value r, Value *o) {
        if constexpr (std::is_same_v<Value, double>) {
            SIMD_512dTYPE vl, v;
            SIMD_512dTYPE cr = _mm512_set1_pd(r);
#if (defined VUNROLL)
#pragma unroll(4)
#endif
            for (int i = 0; i < VECEX_BYTE; i += 8) {
                vl = _mm512_load_pd(&l[i]);
                if constexpr (op == ADD) {
                    v = _mm512_add_pd(vl, cr);
                } else if constexpr (op == SUB) {
                    v = _mm512_sub_pd(vl, cr);
                } else if constexpr (op == MUL) {
                    v = _mm512_mul_pd(vl, cr);
                } else if constexpr (op == DIV) {
                    v = _mm512_div_pd(vl, cr);
                }
                _mm512_store_pd(&o[i], v);
            }
        } else if constexpr (std::is_same_v<Value, int64_t>) {
            SIMD_512iTYPE vl, v;
            SIMD_512iTYPE cr = _mm512_set1_epi64(r);
#if (defined VUNROLL)
#pragma unroll(4)
#endif
            for (int i = 0; i < VECEX_BYTE; i += 8) {
                vl = _mm512_load_epi64(&l[i]);
                if constexpr (op == ADD) {
                    v = _mm512_add_epi64(vl, cr);
                } else if constexpr (op == SUB) {
                    v = _mm512_sub_epi64(vl, cr);
                } else if constexpr (op == MUL) {
#if (defined __AVX512DQ__)
                    v = _mm512_mullo_epi64(vl, cr);
#endif
                } else if constexpr (op == DIV) {
                    // v = _mm512_div_pd(vl, vr);
                }
                _mm512_store_epi64(&o[i], v);
            }
        }
    }

    void evalVVFull(Value *lvalue, Value *rvalue, Value *output, OpType op) {
        switch (op) {
            case ADD:
                vevalOp<ADD>(lvalue, rvalue, output);
                break;
            case SUB:
                vevalOp<SUB>(lvalue, rvalue, output);
                break;
            case MUL:
                vevalOp<MUL>(lvalue, rvalue, output);
                break;
            case DIV:
                vevalOp<DIV>(lvalue, rvalue, output);
                break;
            default:
                break;
        }
    }

    void evalVCFull(Value *vvalue, Value cvalue, Value *output, OpType op) {
        switch (op) {
            case ADD:
                vcevalOp<ADD>(vvalue, cvalue, output);
                break;
            case SUB:
                vcevalOp<SUB>(vvalue, cvalue, output);
                break;
            case MUL:
                vcevalOp<MUL>(vvalue, cvalue, output);
                break;
            case DIV:
                vcevalOp<DIV>(vvalue, cvalue, output);
                break;
            default:
                break;
        }
    }
};

template <typename Value>
class OpPartialEvaluator {
   public:
    OpPartialEvaluator(int _vecSize, int *_selectionVector)
        : vecSize(_vecSize), selectionVector(_selectionVector) {
        data = new (std::align_val_t{VECEX_SIZE}) Value[vecSize];
    }
    ~OpPartialEvaluator() { delete data; }

    Value *evaluate(OpTree *tree, data64 **bufArray) {
        Value *lvalue;
        Value *rvalue;

        if (tree->left->evalType == OP) {
            if (tree->left->type == DOUBLE) {
                OpPartialEvaluator<double> ope1;
                lvalue = (Value *)ope1.evaluate(tree->left, bufArray);
            } else if (tree->left->type == INT) {
                OpPartialEvaluator<int64_t> ope1;
                lvalue = (Value *)ope1.evaluate(tree->left, bufArray);
            }

            if (tree->right->evalType == OP) {
                if (tree->right->type == DOUBLE) {
                    OpPartialEvaluator<double> ope2;
                    rvalue = (Value *)ope2.evaluate(tree->left, bufArray);
                } else if (tree->right->type == INT) {
                    OpPartialEvaluator<int64_t> ope2;
                    rvalue = (Value *)ope2.evaluate(tree->left, bufArray);
                }

                evalOOFull(lvalue, rvalue, data, tree->opType);
            } else if (tree->right->evalType == VAR) {
                rvalue =
                    reinterpret_cast<Value *>(bufArray[tree->right->varKey]);
                evalOVPartial(lvalue, rvalue, data, tree->opType);
            } else {
                if (tree->type == DOUBLE) {
                    evalOCFull(lvalue, tree->right->constData.d, data,
                               tree->opType);
                } else if (tree->type == INT) {
                    evalOCFull(lvalue, tree->right->constData.i, data,
                               tree->opType);
                }
            }
        } else if (tree->left->evalType == VAR) {
            lvalue = reinterpret_cast<Value *>(bufArray[tree->left->varKey]);

            if (tree->right->evalType == OP) {
                if (tree->right->type == DOUBLE) {
                    OpPartialEvaluator<double> ope2;
                    rvalue = (Value *)ope2.evaluate(tree->left, bufArray);
                } else if (tree->right->type == INT) {
                    OpPartialEvaluator<int64_t> ope2;
                    rvalue = (Value *)ope2.evaluate(tree->left, bufArray);
                }

                evalOVPartial(rvalue, lvalue, data, tree->opType);
            } else if (tree->right->evalType == VAR) {
                rvalue =
                    reinterpret_cast<Value *>(bufArray[tree->right->varKey]);
                evalVVPartial(lvalue, rvalue, data, tree->opType);
            } else {
                if (tree->type == DOUBLE) {
                    evalVCPartial(lvalue, tree->right->constData.d, data,
                                  tree->opType);
                } else if (tree->type == INT) {
                    evalVCPartial(lvalue, tree->right->constData.i, data,
                                  tree->opType);
                }
            }
        } else {
            if (tree->right->evalType == OP) {
                if (tree->right->type == DOUBLE) {
                    OpPartialEvaluator<double> ope2;
                    rvalue = (Value *)ope2.evaluate(tree->left, bufArray);
                } else if (tree->right->type == INT) {
                    OpPartialEvaluator<int64_t> ope2;
                    rvalue = (Value *)ope2.evaluate(tree->left, bufArray);
                }

                if (tree->type == DOUBLE) {
                    evalOCFull(tree->left->constData.d, rvalue, data,
                               tree->opType);
                } else if (tree->type == INT) {
                    evalOCFull(tree->left->constData.i, rvalue, data,
                               tree->opType);
                }
            } else if (tree->right->evalType == VAR) {
                rvalue =
                    reinterpret_cast<Value *>(bufArray[tree->right->varKey]);

                if (tree->type == DOUBLE) {
                    evalVCPartial(tree->left->constData.d, rvalue, data,
                                  tree->opType);
                } else if (tree->type == INT) {
                    evalVCPartial(tree->left->constData.i, rvalue, data,
                                  tree->opType);
                }
            }
        }

        return data;
    }

   private:
    int vecSize;
    int *selectionVector;
    Value *data;

    void evalOOFull(Value *lvalue, Value *rvalue, Value *output, OpType op) {
        if constexpr (std::is_same_v<Value, double>) {
            SIMD_512dTYPE vl, vr, v;
            switch (op) {
                case ADD:
                    for (int i = 0; i < vecSize; i += 8) {
                        vl = _mm512_load_pd(&lvalue[i]);
                        vr = _mm512_load_pd(&rvalue[i]);
                        v = _mm512_add_pd(vl, vr);
                        _mm512_store_pd(&output[i], v);
                    }
                    break;
                case SUB:
                    for (int i = 0; i < vecSize; i += 8) {
                        vl = _mm512_load_pd(&lvalue[i]);
                        vr = _mm512_load_pd(&rvalue[i]);
                        v = _mm512_sub_pd(vl, vr);
                        _mm512_store_pd(&output[i], v);
                    }
                    break;
                case MUL:
                    for (int i = 0; i < vecSize; i += 8) {
                        vl = _mm512_load_pd(&lvalue[i]);
                        vr = _mm512_load_pd(&rvalue[i]);
                        v = _mm512_mul_pd(vl, vr);
                        _mm512_store_pd(&output[i], v);
                    }
                    break;
                case DIV:
                    for (int i = 0; i < vecSize; i += 8) {
                        vl = _mm512_load_pd(&lvalue[i]);
                        vr = _mm512_load_pd(&rvalue[i]);
                        v = _mm512_div_pd(vl, vr);
                        _mm512_store_pd(&output[i], v);
                    }
                    break;
                default:
                    break;
            }
        } else if constexpr (std::is_same_v<Value, int64_t>) {
            SIMD_512iTYPE vl, vr, v;
            switch (op) {
                case ADD:
                    for (int i = 0; i < vecSize; i += 8) {
                        vl = _mm512_load_epi64(&lvalue[i]);
                        vr = _mm512_load_epi64(&rvalue[i]);
                        v = _mm512_add_epi64(vl, vr);
                        _mm512_store_epi64(&output[i], v);
                    }
                    break;
                case SUB:
                    for (int i = 0; i < vecSize; i += 8) {
                        vl = _mm512_load_epi64(&lvalue[i]);
                        vr = _mm512_load_epi64(&rvalue[i]);
                        v = _mm512_sub_epi64(vl, vr);
                        _mm512_store_epi64(&output[i], v);
                    }
                    break;
                case MUL:
                    for (int i = 0; i < vecSize; i += 8) {
                        vl = _mm512_load_epi64(&lvalue[i]);
                        vr = _mm512_load_epi64(&rvalue[i]);
                        v = _mm512_mullo_epi64(vl, vr);
                        _mm512_store_epi64(&output[i], v);
                    }
                    break;
                case DIV:
                    // do nothing
                    break;
                default:
                    break;
            }
        }
    }

    template <OpType opt>
    void evalOCFull(Value *ovalue, Value cvalue, Value *output, OpType op) {
        if constexpr (std::is_same_v<Value, double>) {
            SIMD_512dTYPE vl, v;
            SIMD_512dTYPE cr = _mm512_set1_pd(cvalue);
            switch (op) {
                case ADD:
                    for (int i = 0; i < vecSize; i += 8) {
                        vl = _mm512_load_pd(&ovalue[i]);
                        v = _mm512_add_pd(vl, cr);
                        _mm512_store_pd(&output[i], v);
                    }
                    break;
                case SUB:
                    for (int i = 0; i < vecSize; i += 8) {
                        vl = _mm512_load_pd(&ovalue[i]);
                        v = _mm512_sub_pd(vl, cr);
                        _mm512_store_pd(&output[i], v);
                    }
                    break;
                case MUL:
                    for (int i = 0; i < vecSize; i += 8) {
                        vl = _mm512_load_pd(&ovalue[i]);
                        v = _mm512_mul_pd(vl, cr);
                        _mm512_store_pd(&output[i], v);
                    }
                    break;
                case DIV:
                    for (int i = 0; i < vecSize; i += 8) {
                        vl = _mm512_load_pd(&ovalue[i]);
                        v = _mm512_div_pd(vl, cr);
                        _mm512_store_pd(&output[i], v);
                    }
                    break;
                default:
                    break;
            }
        } else if constexpr (std::is_same_v<Value, int64_t>) {
            SIMD_512iTYPE vl, v;
            SIMD_512iTYPE cr = _mm512_set1_pd(cvalue);
            switch (op) {
                case ADD:
                    for (int i = 0; i < vecSize; i += 8) {
                        vl = _mm512_load_epi64(&ovalue[i]);
                        v = _mm512_add_epi64(vl, cr);
                        _mm512_store_epi64(&output[i], v);
                    }
                    break;
                case SUB:
                    for (int i = 0; i < vecSize; i += 8) {
                        vl = _mm512_load_epi64(&ovalue[i]);
                        v = _mm512_sub_epi64(vl, cr);
                        _mm512_store_epi64(&output[i], v);
                    }
                    break;
                case MUL:
                    for (int i = 0; i < vecSize; i += 8) {
                        vl = _mm512_load_epi64(&ovalue[i]);
                        v = _mm512_mullo_epi64(vl, cr);
                        _mm512_store_epi64(&output[i], v);
                    }
                    break;
                case DIV:
                    // do nothing
                    break;
                default:
                    break;
            }
        }
    }

    void evalOVPartial(Value *ovalue, Value *vvalue, Value *output, OpType op) {
        if constexpr (std::is_same_v<Value, double>) {
            SIMD_512dTYPE vl, vr, v;
            switch (op) {
                case ADD:
                    for (int i = 0; i < vecSize; i += 8) {
                        SIMD_256iTYPE sv =
                            _mm256_load_epi32(&selectionVector[i]);
                        vl = _mm512_i32gather_pd(sv, vvalue, 1);
                        vr = _mm512_load_pd(&ovalue[i]);
                        v = _mm512_add_pd(vl, vr);
                        _mm512_store_pd(&output[i], v);
                    }
                    break;
                case SUB:
                    for (int i = 0; i < vecSize; i += 8) {
                        SIMD_256iTYPE sv =
                            _mm256_load_epi32(&selectionVector[i]);
                        vl = _mm512_i32gather_pd(sv, vvalue, 1);
                        vr = _mm512_load_pd(&ovalue[i]);
                        v = _mm512_sub_pd(vl, vr);
                        _mm512_store_pd(&output[i], v);
                    }
                    break;
                case MUL:
                    for (int i = 0; i < vecSize; i += 8) {
                        SIMD_256iTYPE sv =
                            _mm256_load_epi32(&selectionVector[i]);
                        vl = _mm512_i32gather_pd(sv, vvalue, 1);
                        vr = _mm512_load_pd(&ovalue[i]);
                        v = _mm512_mul_pd(vl, vr);
                        _mm512_store_pd(&output[i], v);
                    }
                    break;
                case DIV:
                    for (int i = 0; i < vecSize; i += 8) {
                        SIMD_256iTYPE sv =
                            _mm256_load_epi32(&selectionVector[i]);
                        vl = _mm512_i32gather_pd(sv, vvalue, 1);
                        vr = _mm512_load_pd(&ovalue[i]);
                        v = _mm512_div_pd(vl, vr);
                        _mm512_store_pd(&output[i], v);
                    }
                    break;
                default:
                    break;
            }
        } else if constexpr (std::is_same_v<Value, int64_t>) {
            SIMD_512iTYPE vl, vr, v;
            switch (op) {
                case ADD:
                    for (int i = 0; i < vecSize; i += 8) {
                        SIMD_256iTYPE sv =
                            _mm256_load_epi32(&selectionVector[i]);
                        vl = _mm512_i32gather_epi64(sv, vvalue, 1);
                        vr = _mm512_load_epi64(&ovalue[i]);
                        v = _mm512_add_epi64(vl, vr);
                        _mm512_store_epi64(&output[i], v);
                    }
                    break;
                case SUB:
                    for (int i = 0; i < vecSize; i += 8) {
                        SIMD_256iTYPE sv =
                            _mm256_load_epi32(&selectionVector[i]);
                        vl = _mm512_i32gather_epi64(sv, vvalue, 1);
                        vr = _mm512_load_epi64(&ovalue[i]);
                        v = _mm512_sub_epi64(vl, vr);
                        _mm512_store_epi64(&output[i], v);
                    }
                    break;
                case MUL:
                    for (int i = 0; i < vecSize; i += 8) {
                        SIMD_256iTYPE sv =
                            _mm256_load_epi32(&selectionVector[i]);
                        vl = _mm512_i32gather_epi64(sv, vvalue, 1);
                        vr = _mm512_load_epi64(&ovalue[i]);
                        v = _mm512_mullo_epi64(vl, vr);
                        _mm512_store_epi64(&output[i], v);
                    }
                    break;
                case DIV:
                    // do nothing
                    break;
                default:
                    break;
            }
        }
    }

    void evalVVPartial(Value *lvalue, Value *rvalue, Value *output, OpType op) {
        if constexpr (std::is_same_v<Value, double>) {
            SIMD_512dTYPE vl, vr, v;
            switch (op) {
                case ADD:
                    for (int i = 0; i < vecSize; i += 8) {
                        SIMD_256iTYPE sv =
                            _mm256_load_epi32(&selectionVector[i]);
                        vl = _mm512_i32gather_pd(sv, lvalue, 1);
                        vr = _mm512_i32gather_pd(sv, rvalue, 1);
                        v = _mm512_add_pd(vl, vr);
                        _mm512_store_pd(&output[i], v);
                    }
                    break;
                case SUB:
                    for (int i = 0; i < vecSize; i += 8) {
                        SIMD_256iTYPE sv =
                            _mm256_load_epi32(&selectionVector[i]);
                        vl = _mm512_i32gather_pd(sv, lvalue, 1);
                        vr = _mm512_i32gather_pd(sv, rvalue, 1);
                        v = _mm512_sub_pd(vl, vr);
                        _mm512_store_pd(&output[i], v);
                    }
                    break;
                case MUL:
                    for (int i = 0; i < vecSize; i += 8) {
                        SIMD_256iTYPE sv =
                            _mm256_load_epi32(&selectionVector[i]);
                        vl = _mm512_i32gather_pd(sv, lvalue, 1);
                        vr = _mm512_i32gather_pd(sv, rvalue, 1);
                        v = _mm512_mul_pd(vl, vr);
                        _mm512_store_pd(&output[i], v);
                    }
                    break;
                case DIV:
                    for (int i = 0; i < vecSize; i += 8) {
                        SIMD_256iTYPE sv =
                            _mm256_load_epi32(&selectionVector[i]);
                        vl = _mm512_i32gather_pd(sv, lvalue, 1);
                        vr = _mm512_i32gather_pd(sv, rvalue, 1);
                        v = _mm512_div_pd(vl, vr);
                        _mm512_store_pd(&output[i], v);
                    }
                    break;
                default:
                    break;
            }
        } else if constexpr (std::is_same_v<Value, int64_t>) {
            SIMD_512iTYPE vl, vr, v;
            switch (op) {
                case ADD:
                    for (int i = 0; i < vecSize; i += 8) {
                        SIMD_256iTYPE sv =
                            _mm256_load_epi32(&selectionVector[i]);
                        vl = _mm512_i32gather_epi64(sv, lvalue, 1);
                        vr = _mm512_i32gather_epi64(sv, rvalue, 1);
                        v = _mm512_add_epi64(vl, vr);
                        _mm512_store_epi64(&output[i], v);
                    }
                    break;
                case SUB:
                    for (int i = 0; i < vecSize; i += 8) {
                        SIMD_256iTYPE sv =
                            _mm256_load_epi32(&selectionVector[i]);
                        vl = _mm512_i32gather_epi64(sv, lvalue, 1);
                        vr = _mm512_i32gather_epi64(sv, rvalue, 1);
                        v = _mm512_sub_epi64(vl, vr);
                        _mm512_store_epi64(&output[i], v);
                    }
                    break;
                case MUL:
                    for (int i = 0; i < vecSize; i += 8) {
                        SIMD_256iTYPE sv =
                            _mm256_load_epi32(&selectionVector[i]);
                        vl = _mm512_i32gather_epi64(sv, lvalue, 1);
                        vr = _mm512_i32gather_epi64(sv, rvalue, 1);
                        v = _mm512_mullo_epi64(vl, vr);
                        _mm512_store_epi64(&output[i], v);
                    }
                    break;
                case DIV:
                    // do nothing
                    break;
                default:
                    break;
            }
        }
    }

    void evalVCPartial(Value *vvalue, Value cvalue, Value *output, OpType op) {
        if constexpr (std::is_same_v<Value, double>) {
            SIMD_512dTYPE vl, v;
            SIMD_512dTYPE cr = _mm512_set1_pd(cvalue);
            switch (op) {
                case ADD:
                    for (int i = 0; i < vecSize; i += 8) {
                        SIMD_256iTYPE sv =
                            _mm256_load_epi32(&selectionVector[i]);
                        vl = _mm512_i32gather_pd(sv, vvalue, 1);
                        v = _mm512_add_pd(vl, cr);
                        _mm512_store_pd(&output[i], v);
                    }
                    break;
                case SUB:
                    for (int i = 0; i < vecSize; i += 8) {
                        SIMD_256iTYPE sv =
                            _mm256_load_epi32(&selectionVector[i]);
                        vl = _mm512_i32gather_pd(sv, vvalue, 1);
                        v = _mm512_sub_pd(vl, cr);
                        _mm512_store_pd(&output[i], v);
                    }
                    break;
                case MUL:
                    for (int i = 0; i < vecSize; i += 8) {
                        SIMD_256iTYPE sv =
                            _mm256_load_epi32(&selectionVector[i]);
                        vl = _mm512_i32gather_pd(sv, vvalue, 1);
                        v = _mm512_mul_pd(vl, cr);
                        _mm512_store_pd(&output[i], v);
                    }
                    break;
                case DIV:
                    for (int i = 0; i < vecSize; i += 8) {
                        SIMD_256iTYPE sv =
                            _mm256_load_epi32(&selectionVector[i]);
                        vl = _mm512_i32gather_pd(sv, vvalue, 1);
                        v = _mm512_div_pd(vl, cr);
                        _mm512_store_pd(&output[i], v);
                    }
                    break;
                default:
                    break;
            }
        } else if constexpr (std::is_same_v<Value, int64_t>) {
            SIMD_512iTYPE vl, v;
            SIMD_512iTYPE cr = _mm512_set1_epi64(cvalue);
            switch (op) {
                case ADD:
                    for (int i = 0; i < vecSize; i += 8) {
                        SIMD_256iTYPE sv =
                            _mm256_load_epi32(&selectionVector[i]);
                        vl = _mm512_i32gather_pd(sv, vvalue, 1);
                        v = _mm512_add_epi64(vl, cr);
                        _mm512_store_epi64(&output[i], v);
                    }
                    break;
                case SUB:
                    for (int i = 0; i < vecSize; i += 8) {
                        SIMD_256iTYPE sv =
                            _mm256_load_epi32(&selectionVector[i]);
                        vl = _mm512_i32gather_epi64(sv, vvalue, 1);
                        v = _mm512_sub_epi64(vl, cr);
                        _mm512_store_epi64(&output[i], v);
                    }
                    break;
                case MUL:
                    for (int i = 0; i < vecSize; i += 8) {
                        SIMD_256iTYPE sv =
                            _mm256_load_epi32(&selectionVector[i]);
                        vl = _mm512_i32gather_epi64(sv, vvalue, 1);
                        v = _mm512_mullo_epi64(vl, cr);
                        _mm512_store_epi64(&output[i], v);
                    }
                    break;
                case DIV:
                    // do nothing
                    break;
                default:
                    break;
            }
        }
    }
};
}  // namespace vlex
