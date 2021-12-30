#include "example/JSON/categories_dfa.hpp"
#include "src/common.hpp"
#include "src/converter.hpp"

int main(int argc, char** argv) {
    int stateSize = 31;
    ST_TYPE** dfa = (ST_TYPE**)malloc(sizeof(ST_TYPE*) * stateSize);
    for (int i = 0; i < stateSize; i++) {
        dfa[i] = (ST_TYPE*)malloc(sizeof(ST_TYPE) * 128);
    }
    int acceptStateSize = 1;
    ST_TYPE* acceptStates = (ST_TYPE*)malloc(sizeof(ST_TYPE) * acceptStateSize);
    acceptStates[0] = 28;

    std::string filename = "vfa.hpp";

    generate_sample_dfa(dfa, stateSize);
    VectFA vfa(dfa, acceptStates, stateSize, acceptStateSize);
    vfa.codegen(filename);
}
