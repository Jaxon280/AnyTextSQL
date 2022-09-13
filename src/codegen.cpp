#include "scanner/codegen.hpp"

using namespace vlex;

void Codegen::add_preprocess() {
    code += "#include <string.h>\n";
    code += "#include <string>\n";
    code += "#include <vector>\n";
    code += "#include \"x86intrin.h\"\n\n";

    code += "#define ST_TYPE uint8_t\n";
    code += "#define SIMD_TEXTTYPE __m128i\n";
    code += "#define SIMD_BYTES 16\n";
    code += "#define NUMSTATES ";
    code += std::to_string(qlabels.size());
    code += "\n\n";
}

void Codegen::add_tokenClass() {
    code += "class Token {\n";
    code += "\tstd::string str;\n";
    code += "\tint tokenType = 0;\n";
    code += "public:\n";
    code += "\tvoid set_literals(uint8_t *data, int start, int size) {\n";
    code += "\t\tchar buf[size + 1];\n";
    code += "\t\tbuf[size] = (char)0;\n";
    code += "\t\tstrncpy(buf, (char *)&data[start], size);\n";
    code += "\t\tstr = std::string(buf);\n";
    code += "\t}\n";
    code += "\tstd::string get_literals() { return str; };\n";
    code += "};\n";
    code += "\n";
}

void Codegen::add_member() {
    code += "\tstruct Context {\n";
    code += "\t\tST_TYPE recentAcceptState;\n";
    code += "\t\tint recentAcceptIndex;\n";
    code += "\t\tint tokenStartIndex;\n";
    code += "\t};\n";
    code += "\n";
    code += "\tVFALexer::Context ctx;\n";
    code += "\tSIMD_TEXTTYPE simd_datas[NUMSTATES];\n";
    code += "\tint simd_sizes[NUMSTATES];\n";
    code += "\n";
    code += "\tuint8_t *data;\n";
    code += "\tint size;\n";
    code += "\tint i;\n";
    code += "\n";
}

void Codegen::add_cmpstr(Delta *d) {
    std::string q = std::to_string(d->startState);

    code += "\t\tconst uint8_t ";
    code += "str_q";
    code += q;
    code += "[16] = ";
    code += "{";
    for (int i = 0; i < 16; i++) {
        if (i < d->str.size()) {
            code += std::to_string((int)d->str[i]);
        } else {
            code += std::to_string(0);
        }

        if (i != 15) {
            code += ", ";
        }
    }

    code += "};\n";

    code += "\t\tsimd_datas[";
    code += q;
    code += "] = ";
    code += "_mm_loadu_si128((SIMD_TEXTTYPE*)";
    code += "str_q";
    code += q;
    code += ");\n";

    code += "\t\tsimd_sizes[";
    code += q;
    code += "] = ";
    code += std::to_string(d->str.size());
    code += ";\n";

    code += "\n";
}

void Codegen::add_constructor() {
    code +=
        "\tVFALexer(uint8_t *data, int start, int size) : data(data), "
        "size(size) {\n";
    code +=
        "\t\tctx.tokenStartIndex = start, ctx.recentAcceptIndex = 0, "
        "ctx.recentAcceptState = 0;\n";
    code += "\t\ti = start;\n\n";

    for (Qlabel &ql : qlabels) {
        if (ql.kind == ORDERED || ql.kind == ANY || ql.kind == RANGES) {
            add_cmpstr(ql.delta);
        }
    }

    code += "\t}\n\n";
}

void Codegen::add_generate_token() {
    code +=
        "\tvoid generate_token(std::vector<Token>& tokenVec, ST_TYPE "
        "state, uint8_t *data, int start, int end) {\n";
    code += "\t\tToken token;\n";
    code += "\t\ttoken.set_literals(data, start, end - start);\n";
    code += "\t\t// token.set_type(state);\n";
    code += "\t\ttokenVec.push_back(token);\n";
    code += "\t}\n\n";
}

void Codegen::add_lex() {
    code += "\tstd::vector<Token> lex() {\n";
    code += "\t\tstd::vector<Token> tokenVec;\n";
    code += "\t\tint r = 0;\n";
    code += "\t\tSIMD_TEXTTYPE text;\n";
    code += "\n";

    for (int i = 0; i < qlabels.size(); i++) {
        Qlabel ql = qlabels[i];
        std::string q = std::to_string(i);

        code += "\tq";
        code += q;
        code += ":\n";
        if (i == INIT_STATE) {
            code += "\t\tif (i >= size) goto end;\n";
        }
        if (ql.isAccept) {
            code += "\t\tctx.recentAcceptState = ";
            code += q;
            code += ";\n";
            code += "\t\tctx.recentAcceptIndex = i;\n";
        }

        if (ql.kind != C && ql.kind != INV) {
            add_intrisic(ql, q);
        }

        if (ql.kind == ORDERED || ql.kind == ANY || ql.kind == RANGES ||
            ql.kind == CMPEQ) {
            // add immediate
            code += "\t\tif (r == 16) {\n";
            code += "\t\t\ti += 16;\n";
            code += "\t\t\tgoto q";
            code += q;
            code += ";\n";
            code += "\t\t}\n";
            code += "\t\ti += r;\n";
        }

        add_trans(ql, q);

        if (ql.kind == ORDERED) {
            code += "\t\ti += ";
            code += std::to_string(ql.delta->str.size());
            code += ";\n";
        }
        if (i == INIT_STATE) {
            code +=
                "\t\tctx.recentAcceptState = 0, ctx.recentAcceptIndex = 0;\n";
            code += "\t\tctx.tokenStartIndex = i;\n ";
        }
    }

    add_inv();
    code += "\tend:\n";
    // code += "\t\t}\n\n";
    code += "\t\treturn tokenVec;\n";
    code += "\t}\n";
}

void Codegen::add_intrisic(Qlabel &label, std::string q) {
    code += "\t\ttext = _mm_loadu_si128((SIMD_TEXTTYPE*)(&data[i]));\n";
    code += "\t\tr = _mm_cmpestri(simd_datas[";
    code += q;
    code += "], simd_sizes[";
    code += q;
    if (label.kind == ORDERED) {
        code += "], text, 16, _SIDD_CMP_EQUAL_ORDERED);\n";
    } else if (label.kind == ANY) {
        code += "], text, 16, _SIDD_CMP_EQUAL_ANY);\n";
    } else if (label.kind == RANGES) {
        code += "], text, 16, _SIDD_CMP_RANGES);\n";
    }
}

void Codegen::add_trans(Qlabel &label, std::string q) {
    if (label.kind == ORDERED) {
        std::string backStr = label.delta->backStr;
        int backStr_size = backStr.size();
        if (backStr_size > 0) {
            code += "\t\tif (";

            int j = 1;
            for (char c : backStr) {
                if (j > 1) code += " || ";
                code += "data[i-";
                code += std::to_string(j);
                code += "]!='";
                code += backStr[backStr_size - j];
                code += "'";
                j++;
            }

            code += ") {\n";
            code += "\t\t\ti++;\n";
            code += "\t\t\tgoto q";
            code += std::to_string(label.delta->startState);
            code += ";\n";
            code += "\t\t}\n";
        }
        code += "\t\tif (r > ";
        int length = 16 - label.delta->str.size();
        code += std::to_string(length);
        code += ") goto q";
        code += q;
        code += ";\n";
    } else if (label.kind == ANY || label.kind == RANGES ||
               label.kind == CMPEQ || label.kind == C) {
        code += "\t\tswitch (data[i++]) {\n";

        std::map<ST_TYPE, std::vector<int>> state2char;
        int max = 0, argmax = 0;
        for (int i = 0; i < label.delta->charTable.size(); i++) {
            ST_TYPE s = label.delta->charTable[i];
            state2char[s].push_back(i);
            if (state2char[s].size() > max) {
                max = state2char[s].size();
                argmax = s;
            }
        }

        for (const auto &[key, value] : state2char) {
            if (key != argmax) {
                for (int s : value) {
                    code += "\t\t\tcase ";
                    code += std::to_string(s);
                    code += ":\n";
                    code += "\t\t\t\tgoto q";
                    code += std::to_string(key);
                    code += ";\n";
                    code += "\t\t\t\tbreak;\n";
                }
            }
        }
        code += "\t\t\tdefault:\n";
        code += "\t\t\t\tgoto q";
        code += std::to_string(argmax);
        code += ";\n";
        code += "\t\t\t\tbreak;\n";

        code += "\t\t}\n";
    }
}

void Codegen::add_inv() {
    ST_TYPE state = INV_STATE;
    std::string q = std::to_string(state);
    code += "\tq";
    code += q;
    code += ":\n";

    code += "\t\tif (ctx.recentAcceptState) {\n";
    code +=
        "\t\t\tgenerate_token(tokenVec, ctx.recentAcceptState, "
        "data, ";
    code += "ctx.tokenStartIndex, i);\n";
    code += "\t\t\ti = ctx.recentAcceptIndex + 1;\n";
    code += "\t\t}\n";
    // omit update of tokenStartIndex

    code += "\t\tgoto q1;\n";
}

void Codegen::add_vfaClass() {
    code += "class VFALexer {\n";
    add_member();
    add_generate_token();
    code += "public:\n";
    add_constructor();
    add_lex();
    code += "};\n";
}

Codegen::Codegen(const std::string &filename, std::vector<Qlabel> &qlabels,
                 std::set<ST_TYPE> &states)
    : filename(filename), qlabels(qlabels), states(states) {
    fd = open(filename.c_str(), O_RDWR | O_TRUNC | O_CREAT, 0644);
    if (fd < 0) {
        perror("open");
        exit(1);
    }
}
Codegen::~Codegen() { close(fd); }

int Codegen::generate() {
    add_preprocess();
    add_tokenClass();
    add_vfaClass();

    int size = write(fd, code.c_str(), code.size());
    if (size < 0) {
        perror("write");
        return -1;
    }
    return 0;
}
