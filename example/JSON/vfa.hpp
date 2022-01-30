#include <string.h>
#include <string>
#include <vector>
#include "x86intrin.h"

#define ST_TYPE uint8_t
#define SIMD_TYPE __m128i
#define SIMD_BYTES 16
#define NUMSTATES 16

class Token {
	std::string str;
	int tokenType = 0;
public:
	void set_literals(char *data, int start, int size) {
		char buf[size + 1];
		buf[size] = (char)0;
		strncpy(buf, &data[start], size);
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

	char *data;
	int size;
	int i;

	void generate_token(std::vector<Token>& tokenVec, ST_TYPE state, char *data, int start, int end) {
		Token token;
		token.set_literals(data, start, end - start);
		// token.set_type(state);
		tokenVec.push_back(token);
	}

public:
	VFALexer(char *data, int start, int size) : data(data), size(size) {
		ctx.tokenStartIndex = start, ctx.recentAcceptIndex = 0, ctx.recentAcceptState = 0;
		i = start;

		const char str_q1[16] = {'"', 'c', 'a', 't', 'e', 'g', 'o', 'r', 'i', 'e', 's', '"', ':', '"', '\0', '\0'};
		simd_datas[1] = _mm_loadu_si128((SIMD_TYPE*)str_q1);
		simd_sizes[1] = 14;

		const char str_q2[16] = {'"', 'R', '\\', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
		simd_datas[2] = _mm_loadu_si128((SIMD_TYPE*)str_q2);
		simd_sizes[2] = 3;

		const char str_q13[16] = {'"', '\\', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
		simd_datas[13] = _mm_loadu_si128((SIMD_TYPE*)str_q13);
		simd_sizes[13] = 2;

	}

	std::vector<Token> lex() {
		std::vector<Token> tokenVec;
		int r = 0;
		SIMD_TYPE text;

	q1:
		if (i >= size) goto end;
		text = _mm_loadu_si128((SIMD_TYPE*)(&data[i]));
		r = _mm_cmpestri(simd_datas[1], simd_sizes[1], text, 16, _SIDD_CMP_EQUAL_ORDERED);
		if (r == 16) {
			i += 16;
			goto q1;
		}
		i += r;
		if (r > 2) goto q1;
		i += 14;
		ctx.recentAcceptState = 0, ctx.recentAcceptIndex = 0;
		ctx.tokenStartIndex = i;
 	q2:
		text = _mm_loadu_si128((SIMD_TYPE*)(&data[i]));
		r = _mm_cmpestri(simd_datas[2], simd_sizes[2], text, 16, _SIDD_CMP_EQUAL_ANY);
		if (r == 16) {
			i += 16;
			goto q2;
		}
		i += r;
		switch (data[i++]) {
			case 34:
				goto q1;
				break;
			case 92:
				goto q3;
				break;
			case 82:
				goto q4;
				break;
			default:
				goto q2;
				break;
		}
	q3:
		switch (data[i++]) {
			case 34:
				goto q0;
				break;
			default:
				goto q2;
				break;
		}
	q4:
		switch (data[i++]) {
			case 34:
				goto q1;
				break;
			case 92:
				goto q3;
				break;
			case 82:
				goto q4;
				break;
			case 101:
				goto q5;
				break;
			default:
				goto q2;
				break;
		}
	q5:
		switch (data[i++]) {
			case 34:
				goto q1;
				break;
			case 92:
				goto q3;
				break;
			case 82:
				goto q4;
				break;
			case 115:
				goto q6;
				break;
			default:
				goto q2;
				break;
		}
	q6:
		switch (data[i++]) {
			case 34:
				goto q1;
				break;
			case 92:
				goto q3;
				break;
			case 82:
				goto q4;
				break;
			case 116:
				goto q7;
				break;
			default:
				goto q2;
				break;
		}
	q7:
		switch (data[i++]) {
			case 34:
				goto q1;
				break;
			case 92:
				goto q3;
				break;
			case 82:
				goto q4;
				break;
			case 97:
				goto q8;
				break;
			default:
				goto q2;
				break;
		}
	q8:
		switch (data[i++]) {
			case 34:
				goto q1;
				break;
			case 92:
				goto q3;
				break;
			case 82:
				goto q4;
				break;
			case 117:
				goto q9;
				break;
			default:
				goto q2;
				break;
		}
	q9:
		switch (data[i++]) {
			case 34:
				goto q1;
				break;
			case 92:
				goto q3;
				break;
			case 82:
				goto q4;
				break;
			case 114:
				goto q10;
				break;
			default:
				goto q2;
				break;
		}
	q10:
		switch (data[i++]) {
			case 34:
				goto q1;
				break;
			case 92:
				goto q3;
				break;
			case 82:
				goto q4;
				break;
			case 97:
				goto q11;
				break;
			default:
				goto q2;
				break;
		}
	q11:
		switch (data[i++]) {
			case 34:
				goto q1;
				break;
			case 92:
				goto q3;
				break;
			case 82:
				goto q4;
				break;
			case 110:
				goto q12;
				break;
			default:
				goto q2;
				break;
		}
	q12:
		switch (data[i++]) {
			case 34:
				goto q1;
				break;
			case 92:
				goto q3;
				break;
			case 82:
				goto q4;
				break;
			case 116:
				goto q13;
				break;
			default:
				goto q2;
				break;
		}
	q13:
		text = _mm_loadu_si128((SIMD_TYPE*)(&data[i]));
		r = _mm_cmpestri(simd_datas[13], simd_sizes[13], text, 16, _SIDD_CMP_EQUAL_ANY);
		if (r == 16) {
			i += 16;
			goto q13;
		}
		i += r;
		switch (data[i++]) {
			case 34:
				goto q14;
				break;
			case 92:
				goto q15;
				break;
			default:
				goto q13;
				break;
		}
	q14:
		ctx.recentAcceptState = 14;
		ctx.recentAcceptIndex = i;
		switch (data[i++]) {
			default:
				goto q0;
				break;
		}
	q15:
		switch (data[i++]) {
			case 34:
				goto q0;
				break;
			default:
				goto q13;
				break;
		}
	q0:
		if (ctx.recentAcceptState) {
			generate_token(tokenVec, ctx.recentAcceptState, data, ctx.tokenStartIndex, i);
			i = ctx.recentAcceptIndex + 1;
		}
		goto q1;
	end:
		return tokenVec;
	}
};
