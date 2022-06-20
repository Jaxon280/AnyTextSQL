#include <string.h>

#include <string>
#include <vector>

#include "x86intrin.h"

#define ST_TYPE uint8_t
#define SIMD_TYPE __m128i
#define SIMD_BYTES 16
#define NUMSTATES 26

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
    SIMD_TYPE simd_datas[NUMSTATES];
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

        const uint8_t str_q2[16] = {0, 44, 0, 0, 0, 0, 0, 0,
                                    0, 0,  0, 0, 0, 0, 0, 0};
        simd_datas[2] = _mm_loadu_si128((SIMD_TYPE *)str_q2);
        simd_sizes[2] = 2;

        const uint8_t str_q3[16] = {0, 44, 0, 0, 0, 0, 0, 0,
                                    0, 0,  0, 0, 0, 0, 0, 0};
        simd_datas[3] = _mm_loadu_si128((SIMD_TYPE *)str_q3);
        simd_sizes[3] = 2;

        const uint8_t str_q4[16] = {0, 44, 0, 0, 0, 0, 0, 0,
                                    0, 0,  0, 0, 0, 0, 0, 0};
        simd_datas[4] = _mm_loadu_si128((SIMD_TYPE *)str_q4);
        simd_sizes[4] = 2;

        const uint8_t str_q6[16] = {0, 34, 71, 0, 0, 0, 0, 0,
                                    0, 0,  0,  0, 0, 0, 0, 0};
        simd_datas[6] = _mm_loadu_si128((SIMD_TYPE *)str_q6);
        simd_sizes[6] = 3;

        const uint8_t str_q11[16] = {0, 34, 0, 0, 0, 0, 0, 0,
                                     0, 0,  0, 0, 0, 0, 0, 0};
        simd_datas[11] = _mm_loadu_si128((SIMD_TYPE *)str_q11);
        simd_sizes[11] = 2;

        const uint8_t str_q13[16] = {0, 44, 0, 0, 0, 0, 0, 0,
                                     0, 0,  0, 0, 0, 0, 0, 0};
        simd_datas[13] = _mm_loadu_si128((SIMD_TYPE *)str_q13);
        simd_sizes[13] = 2;

        const uint8_t str_q14[16] = {0, 44, 0, 0, 0, 0, 0, 0,
                                     0, 0,  0, 0, 0, 0, 0, 0};
        simd_datas[14] = _mm_loadu_si128((SIMD_TYPE *)str_q14);
        simd_sizes[14] = 2;

        const uint8_t str_q15[16] = {0, 44, 0, 0, 0, 0, 0, 0,
                                     0, 0,  0, 0, 0, 0, 0, 0};
        simd_datas[15] = _mm_loadu_si128((SIMD_TYPE *)str_q15);
        simd_sizes[15] = 2;

        const uint8_t str_q16[16] = {0, 44, 0, 0, 0, 0, 0, 0,
                                     0, 0,  0, 0, 0, 0, 0, 0};
        simd_datas[16] = _mm_loadu_si128((SIMD_TYPE *)str_q16);
        simd_sizes[16] = 2;

        const uint8_t str_q20[16] = {0, 44, 0, 0, 0, 0, 0, 0,
                                     0, 0,  0, 0, 0, 0, 0, 0};
        simd_datas[20] = _mm_loadu_si128((SIMD_TYPE *)str_q20);
        simd_sizes[20] = 2;

        const uint8_t str_q21[16] = {0, 44, 0, 0, 0, 0, 0, 0,
                                     0, 0,  0, 0, 0, 0, 0, 0};
        simd_datas[21] = _mm_loadu_si128((SIMD_TYPE *)str_q21);
        simd_sizes[21] = 2;

        const uint8_t str_q22[16] = {0, 44, 0, 0, 0, 0, 0, 0,
                                     0, 0,  0, 0, 0, 0, 0, 0};
        simd_datas[22] = _mm_loadu_si128((SIMD_TYPE *)str_q22);
        simd_sizes[22] = 2;

        const uint8_t str_q23[16] = {0, 44, 0, 0, 0, 0, 0, 0,
                                     0, 0,  0, 0, 0, 0, 0, 0};
        simd_datas[23] = _mm_loadu_si128((SIMD_TYPE *)str_q23);
        simd_sizes[23] = 2;
    }

    std::vector<Token> lex() {
        std::vector<Token> tokenVec;
        int r = 0;
        SIMD_TYPE text;

    q1:
        if (i >= size) goto end;
        switch (data[i++]) {
            case 0:
                goto end;
            case 10:
                ctx.recentAcceptState = 0, ctx.recentAcceptIndex = 0;
                ctx.tokenStartIndex = i;
                goto q2;
                break;
            default:
                goto q1;
                break;
        }
    q2:
        text = _mm_loadu_si128((SIMD_TYPE *)(&data[i]));
        r = _mm_cmpestri(simd_datas[2], simd_sizes[2], text, 16,
                         _SIDD_CMP_EQUAL_ANY);
        if (r == 16) {
            i += 16;
            goto q2;
        }
        i += r;
        switch (data[i++]) {
            case 0:
                goto end;
                break;
            case 44:
                goto q3;
                break;
            default:
                goto q2;
                break;
        }
    q3:
        text = _mm_loadu_si128((SIMD_TYPE *)(&data[i]));
        r = _mm_cmpestri(simd_datas[3], simd_sizes[3], text, 16,
                         _SIDD_CMP_EQUAL_ANY);
        if (r == 16) {
            i += 16;
            goto q3;
        }
        i += r;
        switch (data[i++]) {
            case 0:
                goto end;
                break;
            case 44:
                goto q4;
                break;
            default:
                goto q3;
                break;
        }
    q4:
        text = _mm_loadu_si128((SIMD_TYPE *)(&data[i]));
        r = _mm_cmpestri(simd_datas[4], simd_sizes[4], text, 16,
                         _SIDD_CMP_EQUAL_ANY);
        if (r == 16) {
            i += 16;
            goto q4;
        }
        i += r;
        switch (data[i++]) {
            case 0:
                goto end;
                break;
            case 44:
                goto q5;
                break;
            default:
                goto q4;
                break;
        }
    q5:
        switch (data[i++]) {
            case 34:
                goto q6;
                break;
            default:
                goto q0;
                break;
        }
    q6:
        text = _mm_loadu_si128((SIMD_TYPE *)(&data[i]));
        r = _mm_cmpestri(simd_datas[6], simd_sizes[6], text, 16,
                         _SIDD_CMP_EQUAL_ANY);
        if (r == 16) {
            i += 16;
            goto q6;
        }
        i += r;
        switch (data[i++]) {
            case 0:
                goto end;
                break;
            case 71:
                goto q7;
                break;
            case 34:
                goto q19;
                break;
            default:
                goto q6;
                break;
        }
    q7:
        switch (data[i++]) {
            case 0:
                goto end;
                break;
            case 114:
                goto q8;
                break;
            default:
                goto q6;
                break;
        }
    q8:
        switch (data[i++]) {
            case 0:
                goto end;
                break;
            case 101:
                goto q9;
                break;
            default:
                goto q6;
                break;
        }
    q9:
        switch (data[i++]) {
            case 0:
                goto end;
                break;
            case 97:
                goto q10;
                break;
            default:
                goto q6;
                break;
        }
    q10:
        switch (data[i++]) {
            case 0:
                goto end;
                break;
            case 116:
                goto q11;
                break;
            default:
                goto q6;
                break;
        }
    q11:
        text = _mm_loadu_si128((SIMD_TYPE *)(&data[i]));
        r = _mm_cmpestri(simd_datas[11], simd_sizes[11], text, 16,
                         _SIDD_CMP_EQUAL_ANY);
        if (r == 16) {
            i += 16;
            goto q11;
        }
        i += r;
        switch (data[i++]) {
            case 0:
                goto end;
                break;
            case 34:
                goto q12;
                break;
            default:
                goto q11;
                break;
        }
    q12:
        switch (data[i++]) {
            case 34:
                goto q11;
                break;
            case 44:
                goto q13;
                break;
            default:
                goto q0;
                break;
        }
    q13:
        text = _mm_loadu_si128((SIMD_TYPE *)(&data[i]));
        r = _mm_cmpestri(simd_datas[13], simd_sizes[13], text, 16,
                         _SIDD_CMP_EQUAL_ANY);
        if (r == 16) {
            i += 16;
            goto q13;
        }
        i += r;
        switch (data[i++]) {
            case 0:
                goto end;
                break;
            case 44:
                goto q14;
                break;
            default:
                goto q13;
                break;
        }
    q14:
        text = _mm_loadu_si128((SIMD_TYPE *)(&data[i]));
        r = _mm_cmpestri(simd_datas[14], simd_sizes[14], text, 16,
                         _SIDD_CMP_EQUAL_ANY);
        if (r == 16) {
            i += 16;
            goto q14;
        }
        i += r;
        switch (data[i++]) {
            case 0:
                goto end;
                break;
            case 44:
                goto q15;
                break;
            default:
                goto q14;
                break;
        }
    q15:
        text = _mm_loadu_si128((SIMD_TYPE *)(&data[i]));
        r = _mm_cmpestri(simd_datas[15], simd_sizes[15], text, 16,
                         _SIDD_CMP_EQUAL_ANY);
        if (r == 16) {
            i += 16;
            goto q15;
        }
        i += r;
        switch (data[i++]) {
            case 0:
                goto end;
                break;
            case 44:
                goto q16;
                break;
            default:
                goto q15;
                break;
        }
    q16:
        text = _mm_loadu_si128((SIMD_TYPE *)(&data[i]));
        r = _mm_cmpestri(simd_datas[16], simd_sizes[16], text, 16,
                         _SIDD_CMP_EQUAL_ANY);
        if (r == 16) {
            i += 16;
            goto q16;
        }
        i += r;
        switch (data[i++]) {
            case 0:
                goto end;
                break;
            case 44:
                goto q17;
                break;
            default:
                goto q16;
                break;
        }
    q17:
        ctx.recentAcceptState = 17;
        ctx.recentAcceptIndex = i;
        switch (data[i++]) {
            case 0:
                goto q0;
                break;
            case 10:
                goto q2;
                break;
            default:
                goto q17;
                break;
        }
    q18:
        switch (data[i++]) {
            default:
                goto q0;
                break;
        }
    q19:
        switch (data[i++]) {
            case 34:
                goto q6;
                break;
            case 44:
                goto q20;
                break;
            default:
                goto q0;
                break;
        }
    q20:
        text = _mm_loadu_si128((SIMD_TYPE *)(&data[i]));
        r = _mm_cmpestri(simd_datas[20], simd_sizes[20], text, 16,
                         _SIDD_CMP_EQUAL_ANY);
        if (r == 16) {
            i += 16;
            goto q20;
        }
        i += r;
        switch (data[i++]) {
            case 0:
                goto end;
                break;
            case 44:
                goto q21;
                break;
            default:
                goto q20;
                break;
        }
    q21:
        text = _mm_loadu_si128((SIMD_TYPE *)(&data[i]));
        r = _mm_cmpestri(simd_datas[21], simd_sizes[21], text, 16,
                         _SIDD_CMP_EQUAL_ANY);
        if (r == 16) {
            i += 16;
            goto q21;
        }
        i += r;
        switch (data[i++]) {
            case 0:
                goto end;
                break;
            case 44:
                goto q22;
                break;
            default:
                goto q21;
                break;
        }
    q22:
        text = _mm_loadu_si128((SIMD_TYPE *)(&data[i]));
        r = _mm_cmpestri(simd_datas[22], simd_sizes[22], text, 16,
                         _SIDD_CMP_EQUAL_ANY);
        if (r == 16) {
            i += 16;
            goto q22;
        }
        i += r;
        switch (data[i++]) {
            case 0:
                goto end;
                break;
            case 44:
                goto q23;
                break;
            default:
                goto q22;
                break;
        }
    q23:
        text = _mm_loadu_si128((SIMD_TYPE *)(&data[i]));
        r = _mm_cmpestri(simd_datas[23], simd_sizes[23], text, 16,
                         _SIDD_CMP_EQUAL_ANY);
        if (r == 16) {
            i += 16;
            goto q23;
        }
        i += r;
        switch (data[i++]) {
            case 0:
                goto end;
                break;
            case 44:
                goto q24;
                break;
            default:
                goto q23;
                break;
        }
    q24:
        switch (data[i++]) {
            case 0:
                goto end;
                break;
            case 10:
                goto q2;
                break;
            case 34:
                goto q0;
                break;
            case 44:
                goto q0;
                break;
            default:
                goto q24;
                break;
        }
    q25:
        switch (data[i++]) {
            default:
                goto q0;
                break;
        }
    q0:
        if (ctx.recentAcceptState) {
            generate_token(tokenVec, ctx.recentAcceptState, (uint8_t *)data,
                           ctx.tokenStartIndex, i);
            i = ctx.recentAcceptIndex + 1;
            ctx.recentAcceptState = 0, ctx.recentAcceptIndex = 0;
        }
        goto q1;
    end:
        return tokenVec;
    }
};
