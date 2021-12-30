#include "../include/codegen.hpp"

#include "../include/common.hpp"
#include "../include/interface.hpp"

using namespace vlex;

void Codegen::add_preprocess() {
    std::string header = "\"common.hpp\"";
    code += "#include ";
    code += header;
    code += "\n\n";

    code += "#define NUMSTATES ";
    code += std::to_string(qlabels.size());
    code += "\n\n";
}

void Codegen::add_tokenClass() {
    code += "class Token {\n";
    code += "\tstd::string str;\n";
    code += "\tint tokenType = 0;\n";
    code += "public:\n";
    code += "\tvoid set_literals(char *data, int start, int size) {\n";
    code += "\t\tchar buf[size + 1];\n";
    code += "\t\tbuf[size] = (char)0;\n";
    code += "\t\tstrncpy(buf, &data[start], size);\n";
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
    code += "\tSIMD_TYPE simd_datas[NUMSTATES];\n";
    code += "\tint simd_sizes[NUMSTATES];\n";
    code += "\n";
    code += "\tchar *data;\n";
    code += "\tint size;\n";
    code += "\tint i;\n";
    code += "\n";
}

void Codegen::add_cmpstr(Delta *d) {
    std::string q = std::to_string(qlabels[d->startState].state);

    code += "\t\tconst char ";
    code += "str_q";
    code += q;
    code += "[16] = ";
    code += "{";
    for (int i = 0; i < 16; i++) {
        code += "'";

        if (i < d->str.size()) {
            code += d->str[i];
            if (d->str[i] == '\\') {
                code += '\\';
            }
        } else {
            code += '\\';
            code += '0';
        }

        code += "'";
        if (i != 15) {
            code += ", ";
        }
    }

    code += "};\n";

    code += "\t\tsimd_datas[";
    code += q;
    code += "] = ";
    code += "_mm_loadu_si128((SIMD_TYPE*)";
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
        "\tVFALexer(char *data, int start, int size) : data(data), "
        "size(size) {\n";
    code +=
        "\t\tctx.tokenStartIndex = start, ctx.recentAcceptIndex = 0, "
        "ctx.recentAcceptState = 0;\n";
    code += "\t\ti = start;\n\n";

    for (ST_TYPE q : states) {
        if (qlabels[q].kind == ORDERED || qlabels[q].kind == ANY) {
            add_cmpstr(qlabels[q].delta);
        }
    }

    code += "\t}\n\n";
}

void Codegen::add_generate_token() {
    code +=
        "\tvoid generate_token(std::vector<Token>& tokenVec, ST_TYPE "
        "state, char *data, int start, int end) {\n";
    code += "\t\tToken token;\n";
    code += "\t\ttoken.set_literals(data, start, end - start);\n";
    code += "\t\t// token.set_type(state);\n";
    code += "\t\ttokenVec.push_back(token);\n";
    code += "\t}\n\n";
}

void Codegen::add_ltable(Qlabel &label) {
    std::string q = std::to_string(label.state);

    code += "\t\tstatic const void* trans_q";
    code += q;
    code += "[] = ";
    code += "{";

    if (label.kind == ORDERED) {
        for (int i = 0; i < 17; i++) {
            code += "&&q";
            code += std::to_string(qlabels[label.delta->r_table[i]].state);
            if (i <= 16 - label.delta->str.size()) {
                code += "_sink";
            }

            if (i != 16) {
                code += ", ";
            }
        }
    } else {
        code += "&&end, ";
        for (int i = 1; i < ASCII_SZ; i++) {
            code += "&&q";
            code += std::to_string(qlabels[label.delta->char_table[i]].state);
            if (label.delta->startState != label.delta->char_table[i] &&
                label.delta->char_table[i] != INV_STATE) {
                code += "_inc";
            }

            if (i != ASCII_SZ - 1) {
                code += ", ";
            }
        }
    }

    code += "};\n";
}

void Codegen::add_lex() {
    code += "\tstd::vector<Token> lex() {\n";
    code += "\t\tstd::vector<Token> tokenVec;\n";
    code += "\t\tint r = 0;\n";
    code += "\t\tSIMD_TYPE text;\n";
    code += "\n";

    for (ST_TYPE s : states) {
        add_ltable(qlabels[s]);
    }

    code += "\n";
    code += "\t\tgoto q1;\n";

    for (auto it = std::next(states.begin()); it != states.end(); it++) {
        ST_TYPE state = *it;
        Qlabel ql = qlabels[state];
        std::string q = std::to_string(ql.state);

        add_qlabel(ql, q);
        add_mmunch(ql);

        if (ql.kind != C && ql.kind != INV) {
            add_intrisic(ql, q);
        }
        if (state == INIT_STATE) {
            code += "\t\tctx.tokenStartIndex = i;\n ";
        }
        add_trans(ql, q);
    }

    add_inv();
    code += "\tend:\n";
    // code += "\t\t}\n\n";
    code += "\t\treturn tokenVec;\n";
    code += "\t}\n";
}

void Codegen::add_inv() {
    ST_TYPE state = INV_STATE;
    std::string q = std::to_string(qlabels[state].state);
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

void Codegen::add_mmunch(Qlabel &label) {
    if (label.state == INIT_STATE) {
        code += "\t\tif (i >= size) goto end;\n";
        code += "\t\tctx.recentAcceptState = 0, ctx.recentAcceptIndex = 0;\n";
    }
    if (label.is_accept) {
        code += "\t\tctx.recentAcceptState = ";
        code += std::to_string(label.state);
        code += ";\n";
        code += "\t\tctx.recentAcceptIndex = i;\n";
    }
}

void Codegen::add_qlabel(Qlabel &label, std::string q) {
    if (label.is_sink) {
        code += "\tq";
        code += q;
        code += "_sink:\n";
        code += "\t\ti += ";
        if (label.is_inc) {
            code += std::to_string(label.c_length - 1);
        } else {
            code += std::to_string(label.c_length);
        }
        code += ";\n";
    }

    if (label.is_inc) {
        code += "\tq";
        code += q;
        code += "_inc:\n";
        code += "\t\ti++;\n";
    }

    code += "\tq";
    code += q;
    code += ":\n";
}

void Codegen::add_intrisic(Qlabel &label, std::string q) {
    code += "\t\ttext = _mm_loadu_si128((SIMD_TYPE*)(&data[i]));\n";
    code += "\t\tr = _mm_cmpestri(simd_datas[";
    code += q;
    code += "], simd_sizes[";
    code += q;
    if (label.kind == ORDERED) {
        code += "], text, 16, _SIDD_CMP_EQUAL_ORDERED);\n";
    } else if (label.kind == ANY) {
        code += "], text, 16, _SIDD_CMP_EQUAL_ANY);\n";
    }
    code += "\t\ti += r;\n";
}

void Codegen::add_trans(Qlabel &label, std::string q) {
    if (label.kind == C || label.kind == ANY) {
        code += "\t\tgoto *trans_q";
        code += q;
        code += "[data[i]];\n";
    } else if (label.kind == ORDERED) {
        code += "\t\tgoto *trans_q";
        code += q;
        code += "[r];\n";
    }
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

Codegen::Codegen(const std::string &filename,
                 std::map<ST_TYPE, Qlabel> &qlabels, std::set<ST_TYPE> &states)
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
