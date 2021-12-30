#include <string.h>
#include <string>
#include <vector>
#include "x86intrin.h"

#define ST_TYPE uint8_t
#define SIMD_TYPE __m128i
#define SIMD_BYTES 16
#define NUMSTATES 18

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

		const char str_q14[16] = {'"', '\\', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};
		simd_datas[14] = _mm_loadu_si128((SIMD_TYPE*)str_q14);
		simd_sizes[14] = 2;

	}

	std::vector<Token> lex() {
		std::vector<Token> tokenVec;
		int r = 0;
		SIMD_TYPE text;

		static const void* trans_q0[] = {&&end, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0};
		static const void* trans_q1[] = {&&q2_sink, &&q2_sink, &&q2_sink, &&q1, &&q1, &&q1, &&q1, &&q1, &&q1, &&q1, &&q1, &&q1, &&q1, &&q1, &&q1, &&q1, &&q1};
		static const void* trans_q2[] = {&&end, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q1_inc, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q5_inc, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q3_inc, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2, &&q2};
		static const void* trans_q3[] = {&&end, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q4_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc};
		static const void* trans_q4[] = {&&end, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q2_inc, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0};
		static const void* trans_q5[] = {&&end, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q5, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q3_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q6_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q0, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc};
		static const void* trans_q6[] = {&&end, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q5_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q3_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q0, &&q7_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc};
		static const void* trans_q7[] = {&&end, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q5_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q3_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q0, &&q2_inc, &&q8_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc};
		static const void* trans_q8[] = {&&end, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q5_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q3_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q9_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q0, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc};
		static const void* trans_q9[] = {&&end, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q5_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q3_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q0, &&q2_inc, &&q2_inc, &&q10_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc};
		static const void* trans_q10[] = {&&end, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q5_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q3_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q11_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc};
		static const void* trans_q11[] = {&&end, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q5_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q3_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q12_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q0, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc};
		static const void* trans_q12[] = {&&end, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q5_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q3_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q13_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q0, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc};
		static const void* trans_q13[] = {&&end, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q5_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q3_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q0, &&q2_inc, &&q14_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc, &&q2_inc};
		static const void* trans_q14[] = {&&end, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q15_inc, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q16_inc, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14, &&q14};
		static const void* trans_q15[] = {&&end, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0};
		static const void* trans_q16[] = {&&end, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q17_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc, &&q14_inc};
		static const void* trans_q17[] = {&&end, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q14_inc, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0, &&q0};

		goto q1;
	q1_inc:
		i++;
	q1:
		if (i >= size) goto end;
		ctx.recentAcceptState = 0, ctx.recentAcceptIndex = 0;
		text = _mm_loadu_si128((SIMD_TYPE*)(&data[i]));
		r = _mm_cmpestri(simd_datas[1], simd_sizes[1], text, 16, _SIDD_CMP_EQUAL_ORDERED);
		i += r;
		ctx.tokenStartIndex = i;
 		goto *trans_q1[r];
	q2_sink:
		i += 13;
	q2_inc:
		i++;
	q2:
		text = _mm_loadu_si128((SIMD_TYPE*)(&data[i]));
		r = _mm_cmpestri(simd_datas[2], simd_sizes[2], text, 16, _SIDD_CMP_EQUAL_ANY);
		i += r;
		goto *trans_q2[data[i]];
	q3_inc:
		i++;
	q3:
		goto *trans_q3[data[i]];
	q4_inc:
		i++;
	q4:
		goto *trans_q4[data[i]];
	q5_inc:
		i++;
	q5:
		goto *trans_q5[data[i]];
	q6_inc:
		i++;
	q6:
		goto *trans_q6[data[i]];
	q7_inc:
		i++;
	q7:
		goto *trans_q7[data[i]];
	q8_inc:
		i++;
	q8:
		goto *trans_q8[data[i]];
	q9_inc:
		i++;
	q9:
		goto *trans_q9[data[i]];
	q10_inc:
		i++;
	q10:
		goto *trans_q10[data[i]];
	q11_inc:
		i++;
	q11:
		goto *trans_q11[data[i]];
	q12_inc:
		i++;
	q12:
		goto *trans_q12[data[i]];
	q13_inc:
		i++;
	q13:
		goto *trans_q13[data[i]];
	q14_inc:
		i++;
	q14:
		text = _mm_loadu_si128((SIMD_TYPE*)(&data[i]));
		r = _mm_cmpestri(simd_datas[14], simd_sizes[14], text, 16, _SIDD_CMP_EQUAL_ANY);
		i += r;
		goto *trans_q14[data[i]];
	q15_inc:
		i++;
	q15:
		ctx.recentAcceptState = 15;
		ctx.recentAcceptIndex = i;
		goto *trans_q15[data[i]];
	q16_inc:
		i++;
	q16:
		goto *trans_q16[data[i]];
	q17_inc:
		i++;
	q17:
		goto *trans_q17[data[i]];
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
