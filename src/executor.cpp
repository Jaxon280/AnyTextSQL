#include "executor.hpp"

using namespace vlex;

Executor::Executor() {}
Executor::Executor(VectFA *vfa, SIZE_TYPE _start) { setVFA(vfa, _start); }

inline void Executor::cmpestri_ord(ST_TYPE cur_state) {
loop:
    if (i >= size) return;
    SIMD_TYPE text = _mm_loadu_si128((SIMD_TYPE *)(&data[i]));
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
    SIMD_TYPE text = _mm_loadu_si128((SIMD_TYPE *)(&data[i]));
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
    SIMD_TYPE text = _mm_loadu_si128((SIMD_TYPE *)(&data[i]));
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

void Executor::generateToken(std::vector<Token> &tokenVec, DATA_TYPE *data,
                             SIZE_TYPE start, SIZE_TYPE end) {
    Token token;
    token.set_literals(data, start, end - start);
    tokenVec.push_back(token);
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

    SIMDDatas = new SIMD_TYPE[stateSize];
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

            SIMDDatas[iter] = _mm_loadu_si128((SIMD_TYPE *)strdata);
            SIMDSizes[iter] = (int)it.delta->str.size();
        }
        iter++;
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
                        // todo: change it when no submatch
                        generateToken(tokenVec, data, subMatches[s].start,
                                      subMatches[s].end);
                    }

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

#if (defined BENCH)
    gettimeofday(&lex2, NULL);
    ex_time =
        (lex2.tv_sec - lex1.tv_sec) + (lex2.tv_usec - lex1.tv_usec) * 0.000001;
    printf("#BENCH_exec: %lf s\n\n", ex_time);
#endif
}
