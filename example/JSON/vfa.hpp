#include <string.h>

#include <string>
#include <vector>

#include "x86intrin.h"

#define ST_TYPE uint8_t
#define SIMD_TEXTTYPE __m128i
#define SIMD_BYTES 16
#define NUMSTATES 6

class Token {
    std::string str;
    int tokenType = 0;

   public:
    void set_literals(uint8_t *data, int start, int size) {
        char buf[size + 1];
        buf[size] = (char)0;
        strncpy(buf, (char *)&data[start], size);
        str = std::string(buf);
    }
    std::string get_literals() { return str; };
};

class VFALexer {
    struct Context {
        ST_TYPE recentAcceptState;
        int recentAcceptIndex;
        int tokenStartIndex;
    };

    VFALexer::Context ctx;
    SIMD_TEXTTYPE simd_datas[NUMSTATES];
    int simd_sizes[NUMSTATES];

    uint8_t *data;
    int size;
    int i;

    void generate_token(std::vector<Token> &tokenVec, ST_TYPE state,
                        uint8_t *data, int start, int end) {
        Token token;
        token.set_literals(data, start, end - start);
        // token.set_type(state);
        tokenVec.push_back(token);
    }

   public:
    VFALexer(uint8_t *data, int start, int size) : data(data), size(size) {
        ctx.tokenStartIndex = start, ctx.recentAcceptIndex = 0,
        ctx.recentAcceptState = 0;
        i = start;

        const uint8_t str_q1[16] = {116, 97, 114, 115, 34, 58, 0, 0,
                                    0,   0,  0,   0,   0,  0,  0, 0};
        simd_datas[1] = _mm_loadu_si128((SIMD_TEXTTYPE *)str_q1);
        simd_sizes[1] = 6;
    }

    std::vector<Token> lex() {
        std::vector<Token> tokenVec;
        int r = 0;
        SIMD_TEXTTYPE text;

    q1:
        if (i >= size) goto end;
        text = _mm_loadu_si128((SIMD_TEXTTYPE *)(&data[i]));
        r = _mm_cmpestri(simd_datas[1], simd_sizes[1], text, 16,
                         _SIDD_CMP_EQUAL_ORDERED);
        if (r == 16) {
            i += 16;
            goto q1;
        }
        i += r;
        if (data[i - 1] != 's' || data[i - 2] != '"') {
            i++;
            goto q1;
        }
        if (r > 10) goto q1;
        i += 6;
        ctx.recentAcceptState = 0, ctx.recentAcceptIndex = 0;
        ctx.tokenStartIndex = i;
    q2:
        switch (data[i++]) {
            case 49:
                goto q3;
                break;
            case 50:
                goto q3;
                break;
            case 51:
                goto q3;
                break;
            case 52:
                goto q3;
                break;
            case 53:
                goto q3;
                break;
            case 54:
                goto q3;
                break;
            case 55:
                goto q3;
                break;
            case 56:
                goto q3;
                break;
            case 57:
                goto q3;
                break;
            default:
                goto q0;
                break;
        }
    q3:
        ctx.recentAcceptState = 3;
        ctx.recentAcceptIndex = i;
        switch (data[i++]) {
            case 48:
                goto q3;
                break;
            case 49:
                goto q3;
                break;
            case 50:
                goto q3;
                break;
            case 51:
                goto q3;
                break;
            case 52:
                goto q3;
                break;
            case 53:
                goto q3;
                break;
            case 54:
                goto q3;
                break;
            case 55:
                goto q3;
                break;
            case 56:
                goto q3;
                break;
            case 57:
                goto q3;
                break;
            case 46:
                goto q4;
                break;
            default:
                goto q0;
                break;
        }
    q4:
        switch (data[i++]) {
            case 48:
                goto q5;
                break;
            case 49:
                goto q5;
                break;
            case 50:
                goto q5;
                break;
            case 51:
                goto q5;
                break;
            case 52:
                goto q5;
                break;
            case 53:
                goto q5;
                break;
            case 54:
                goto q5;
                break;
            case 55:
                goto q5;
                break;
            case 56:
                goto q5;
                break;
            case 57:
                goto q5;
                break;
            default:
                goto q0;
                break;
        }
    q5:
        ctx.recentAcceptState = 5;
        ctx.recentAcceptIndex = i;
        switch (data[i++]) {
            case 48:
                goto q5;
                break;
            case 49:
                goto q5;
                break;
            case 50:
                goto q5;
                break;
            case 51:
                goto q5;
                break;
            case 52:
                goto q5;
                break;
            case 53:
                goto q5;
                break;
            case 54:
                goto q5;
                break;
            case 55:
                goto q5;
                break;
            case 56:
                goto q5;
                break;
            case 57:
                goto q5;
                break;
            default:
                goto q0;
                break;
        }
    q0:
        if (ctx.recentAcceptState) {
            generate_token(tokenVec, ctx.recentAcceptState, data,
                           ctx.tokenStartIndex, i);
            i = ctx.recentAcceptIndex + 1;
        }
        goto q1;
    end:
        return tokenVec;
    }
};
