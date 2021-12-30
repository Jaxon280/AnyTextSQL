#include "common.hpp"

struct Delta {
    ST_TYPE startState;
    std::string str;
    std::vector<ST_TYPE> char_table;
    std::vector<ST_TYPE> r_table;
    // SIMD_KIND inst;
};

struct Qstar {
    ST_TYPE source;
    ST_TYPE sink;
    std::set<ST_TYPE> states;
    std::string str;
};

typedef enum _simd_kind { ORDERED, ANY, RANGES, CMPEQ, C, INV } SIMDKind;

// #define I_INC 0
// #define I_R 1
// #define I_STRLEN 2

struct Qlabel {
    SIMDKind kind;
    // std::set<int> types;
    bool is_sink = false;
    bool is_inc = false;  // delta_any or delta_c
    int c_length = 0;
};

class VectFA {
    std::set<ST_TYPE> states;
    std::set<ST_TYPE> acceptStates;
    int initState;

    std::vector<Delta> delta_ord;
    std::vector<Delta> delta_any;
    std::vector<Delta> delta_ranges;
    std::vector<Delta> delta_c;
    std::vector<Delta> delta_cmpeq;
    // ST_TYPE **dfa;
    std::map<ST_TYPE, std::vector<ST_TYPE>> dfa;
    std::map<ST_TYPE, ST_TYPE> stateMap;
    std::map<ST_TYPE, Qlabel> state2Qlabel;

    std::string code;

    std::vector<ST_TYPE> construct_Qs(int state_sz) {
        std::vector<ST_TYPE> Qs;
        for (ST_TYPE i = INIT_STATE; i < state_sz; i++) {
            int next_c = 0;
            for (int j = 0; j < ASCII_SZ; j++) {
                if (dfa[i][j] != i && i == 1) {
                    next_c++;
                    if (next_c > 1) break;
                }
            }
            if (next_c == 1) Qs.push_back(i);
        }
        return Qs;
    }

    std::vector<Qstar> construct_Qstars(std::vector<ST_TYPE> Qsource) {
        std::vector<Qstar> Qstar_set;
        while (Qsource.size() != 0) {
            ST_TYPE qs = Qsource.back();
            Qsource.pop_back();
            Qstar Qs;
            Qs.source = qs;
            Qs.states.insert(qs);

            ST_TYPE q;
            for (int c = 0; c < ASCII_SZ; c++) {
                if (dfa[qs][c] != qs) {
                    q = dfa[qs][c];
                    Qs.str += (char)c;
                }
            }
            Qs.states.insert(q);
            ST_TYPE qx = qs;

            while (true) {
                ST_TYPE qnext = 0, cnext = 0;
                int N = 0;
                for (int c = 0; c < ASCII_SZ; c++) {
                    ST_TYPE qn = dfa[q][c];
                    if (Qs.states.find(qn) != Qs.states.end() || qn == q) {
                        if (qn != dfa[qx][c]) goto end_dfs;
                    } else {
                        qnext = qn;
                        cnext = c;
                        N++;
                        if (N > 1) goto end_dfs;
                    }
                }
                Qs.states.insert(qnext);
                Qs.sink = qnext;
                Qs.str += (char)cnext;
                // if found in Qsource...
                q = qnext;
                qx = dfa[qx][cnext];
            }
        end_dfs:
            Qstar_set.push_back(Qs);
        }

        return Qstar_set;
    }

    void construct_delta_ords(std::vector<Qstar> Qstar_set) {
        for (Qstar Qs : Qstar_set) {
            Delta new_ord;
            new_ord.startState = Qs.source;
            new_ord.str = Qs.str;
            for (int i = 0; i <= 16; i++) {
                if (i <= 16 - new_ord.str.size()) {
                    new_ord.r_table.push_back(Qs.sink);
                } else {
                    new_ord.r_table.push_back(Qs.source);
                }
            }

            state2Qlabel[Qs.source].kind = ORDERED;
            // state2Qlabel[Qs.source].types.insert(I_R);
            // state2Qlabel[Qs.sink].types.insert(I_STRLEN);
            state2Qlabel[Qs.sink].is_sink = true;
            state2Qlabel[Qs.sink].c_length = new_ord.str.size();

            delta_ord.push_back(new_ord);
        }
    }

    std::set<ST_TYPE> construct_Qtilde(std::set<ST_TYPE> Qstar_source) {
        std::set<ST_TYPE> Qtilde;

        for (ST_TYPE q : states) {
            int count = 0;
            for (int c = 0; c < ASCII_SZ; c++) {
                if (q == dfa[q][c]) {
                    count++;
                }
            }

            if (q != INV_STATE && Qstar_source.find(q) == Qstar_source.end() &&
                count > 1) {  // additional condition: count > 1
                Qtilde.insert(q);
            }
        }
        return Qtilde;
    }

    void construct_delta_anys(std::set<ST_TYPE> Qtilde) {
        for (ST_TYPE q : Qtilde) {
            Delta new_any;
            new_any.startState = q;
            new_any.char_table.resize(ASCII_SZ);

            state2Qlabel[q].kind = ANY;
            // state2Qlabel[q].types.insert(I_R);

            for (int c = 0; c < ASCII_SZ; c++) {
                new_any.char_table[c] = dfa[q][c];
                if (dfa[q][c] != q) {
                    new_any.str += (char)c;
                    // state2Qlabel[dfa[q][c]].types.insert(I_R);
                    state2Qlabel[dfa[q][c]].is_inc = true;
                }
            }

            delta_any.push_back(new_any);
        }
    }

    void construct_delta_cs(std::set<ST_TYPE> Qstar_source,
                            std::set<ST_TYPE> Qtilde) {
        for (ST_TYPE q : states) {
            if (Qstar_source.find(q) == Qstar_source.end() &&
                Qtilde.find(q) == Qtilde.end()) {
                Delta new_c;
                new_c.startState = q;
                new_c.char_table.resize(ASCII_SZ);
                state2Qlabel[q].kind = C;
                for (int c = 0; c < ASCII_SZ; c++) {
                    new_c.char_table[c] = dfa[q][c];
                    // state2Qlabel[dfa[q][c]].types.insert(I_INC);
                    state2Qlabel[dfa[q][c]].is_inc = true;
                }
                delta_c.push_back(new_c);
            }
        }
    }

    void add_preprocess() {
        std::string header = "\"common.hpp\"";
        code += "#include ";
        code += header;
        code += "\n\n";

        code += "#define NUMSTATES ";
        code += std::to_string(states.size());
        code += "\n\n";
    }

    void add_tokenClass() {
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

    void add_member() {
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

    void add_cmpc(Delta d) {
        std::string q = std::to_string(stateMap[d.startState]);

        code += "\t\tconst char ";
        code += "str_q";
        code += q;
        code += "[16] = ";
        code += "{";
        for (int i = 0; i < 16; i++) {
            code += "'";
            if (i < d.str.size()) {
                code += d.str[i];
                if (d.str[i] == '\\') {
                    code += '\\';
                }
            } else {
                code += '\\';
                code += '0';
            }

            code += "'";
            if (i < 15) {
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
        code += std::to_string(d.str.size());
        code += ";\n";

        code += "\n";
    }

    void add_constructor() {
        code +=
            "\tVFALexer(char *data, int start, int size) : data(data), "
            "size(size) {\n";
        code +=
            "\t\tctx.tokenStartIndex = start, ctx.recentAcceptIndex = 0, "
            "ctx.recentAcceptState = 0;\n";
        code += "\t\ti = start;\n\n";

        for (Delta ord : delta_ord) {
            add_cmpc(ord);
        }
        for (Delta any : delta_any) {
            add_cmpc(any);
        }

        code += "\t}\n\n";
    }

    void add_generate_token() {
        code +=
            "\tvoid generate_token(std::vector<Token>& tokenVec, ST_TYPE "
            "state, char *data, int start, int end) {\n";
        code += "\t\tToken token;\n";
        code += "\t\ttoken.set_literals(data, start, end - start);\n";
        code += "\t\t// token.set_type(state);\n";
        code += "\t\ttokenVec.push_back(token);\n";
        code += "\t}\n\n";
    }

    void add_lex() {
        code += "\tstd::vector<Token> lex() {\n";
        code += "\t\tstd::vector<Token> tokenVec;\n";
        code += "\t\tint r = 0;\n";
        code += "\t\tSIMD_TYPE text;\n";
        code += "\n";

        for (Delta ord : delta_ord) {
            std::string q = std::to_string(stateMap[ord.startState]);

            code += "\t\tstatic const void* trans_q";
            code += q;
            code += "[] = ";

            code += "{";
            for (int i = 0; i <= 16; i++) {
                code += "&&q";
                code += std::to_string(stateMap[ord.r_table[i]]);
                if (i <= 16 - ord.str.size()) {
                    code += "_sink";
                }

                if (i != 16) {
                    code += ", ";
                }
            }
            code += "};\n";
        }

        for (Delta any : delta_any) {
            std::string q = std::to_string(stateMap[any.startState]);

            code += "\t\tstatic const void* trans_q";
            code += q;
            code += "[] = ";

            code += "{&&end, ";

            for (int i = 1; i < ASCII_SZ; i++) {
                code += "&&q";
                code += std::to_string(stateMap[any.char_table[i]]);
                if (any.startState != any.char_table[i] &&
                    any.char_table[i] != INV_STATE) {
                    code += "_inc";
                }

                if (i != ASCII_SZ - 1) {
                    code += ", ";
                }
            }
            code += "};\n";
        }

        for (Delta c : delta_c) {
            std::string q = std::to_string(stateMap[c.startState]);

            code += "\t\tstatic const void* trans_q";
            code += q;
            code += "[] = ";

            code += "{&&end, ";
            for (int i = 1; i < ASCII_SZ; i++) {
                code += "&&q";
                code += std::to_string(stateMap[c.char_table[i]]);
                if (c.char_table[i] != INV_STATE) {
                    code += "_inc";
                }

                if (i != ASCII_SZ - 1) {
                    code += ", ";
                }
            }
            code += "};\n";
        }

        code += "\n";
        code += "\t\tgoto q1;\n";
        // code += "\t\twhile (i < size) {\n";

        for (auto it = std::next(states.begin()); it != states.end(); it++) {
            ST_TYPE state = *it;
            std::string q = std::to_string(stateMap[state]);
            Qlabel label = state2Qlabel[state];

            add_qlabel(label, q);
            add_mmunch(label, state);

            if (label.kind != C && label.kind != INV) {
                add_intrisic(label, q);
            }
            if (state == INIT_STATE) {
                code += "\t\tctx.tokenStartIndex = i;\n ";
            }
            add_trans(label, q);
        }

        add_inv();
        code += "\tend:\n";
        // code += "\t\t}\n\n";
        code += "\t\treturn tokenVec;\n";
        code += "\t}\n";
    }

    void add_inv() {
        ST_TYPE state = INV_STATE;
        std::string q = std::to_string(stateMap[state]);
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
        // omit tokenStartIndex

        code += "\t\tgoto q1;\n";
    }

    void add_mmunch(Qlabel label, ST_TYPE state) {
        if (state == INIT_STATE) {
            code += "\t\tif (i >= size) goto end;\n";
            code +=
                "\t\tctx.recentAcceptState = 0, ctx.recentAcceptIndex = 0;\n";
        }
        if (acceptStates.find(state) != acceptStates.end()) {
            code += "\t\tctx.recentAcceptState = ";
            code += std::to_string(stateMap[state]);
            code += ";\n";
            code += "\t\tctx.recentAcceptIndex = i;\n";
        }
    }

    void add_qlabel(Qlabel label, std::string q) {
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

    // void add_qlabel(Qlabel label, std::string q) {
    //     if (label.types.find(I_STRLEN) != label.types.end()) {
    //         code += "\tq";
    //         code += q;
    //         code += "_strlen:\n";
    //         code += "\t\ti += ";
    //         code += std::to_string(label.c_length);
    //         if (label.types.find(I_R) != label.types.end()) {
    //             code += ";\n";

    //             code += "\tq";
    //             code += q;
    //             code += "_r:\n";
    //             code += "\t\ti += r";
    //             if (label.types.find(I_INC) != label.types.end()) {
    //                 code += " - 1;\n";

    //                 code += "\tq";
    //                 code += q;
    //                 code += "_inc:\n";
    //                 code += "\t\ti++;\n";
    //             } else {
    //                 code += ";\n";
    //             }
    //         } else {
    //             code += "+ r";
    //             code += ";\n";
    //         }
    //     } else if (label.types.find(I_R) != label.types.end()) {
    //         code += "\tq";
    //         code += q;
    //         code += "_r:\n";
    //         code += "\t\ti += r";
    //         if (label.types.find(I_INC) != label.types.end()) {
    //             code += " - 1;\n";

    //             code += "\tq";
    //             code += q;
    //             code += "_inc:\n";
    //             code += "\t\ti++;\n";
    //         } else {
    //             code += ";\n";
    //         }
    //     } else {
    //         code += "\tq";
    //         code += q;
    //         code += "_inc:\n";
    //         code += "\t\ti++;\n";
    //     }
    // }

    void add_intrisic(Qlabel label, std::string q) {
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

    void add_trans(Qlabel label, std::string q) {
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

    void add_vfaClass() {
        code += "class VFALexer {\n";
        add_member();
        add_generate_token();
        code += "public:\n";
        add_constructor();
        add_lex();
        code += "};\n";
    }

   public:
    VectFA(ST_TYPE **fa, ST_TYPE *accepts, int stateSize, int acceptStateSize) {
        initState = INIT_STATE;
        for (ST_TYPE i = INV_STATE; i < stateSize; i++) {
            states.insert(i);
        }
        for (ST_TYPE i = INV_STATE; i < stateSize; i++) {
            for (int j = 0; j < ASCII_SZ; j++) {
                dfa[i].push_back(fa[i][j]);
            }
        }
        for (int i = 0; i < acceptStateSize; i++) {
            acceptStates.insert(accepts[i]);
        }

        // std::map<ST_TYPE, std::set<ST_TYPE>> Q_access;
        // for (ST_TYPE i : states) {
        //     for (int j = 0; j < ASCII_SZ; j++) {
        //         Q_access[dfa[i][j]].insert(i);
        //     }
        // }

        std::vector<ST_TYPE> Qs = construct_Qs(stateSize);
        std::vector<Qstar> Qstars = construct_Qstars(Qs);

        std::set<ST_TYPE> Qstar_source;
        for (Qstar Qs : Qstars) {
            Qstar_source.insert(Qs.source);
            for (ST_TYPE q : Qs.states) {
                if (q != Qs.source && q != Qs.sink) {
                    states.erase(q);
                    dfa.erase(q);
                }
            }
        }
        std::set<ST_TYPE> Qtilde = construct_Qtilde(Qstar_source);

        int i = 0;
        for (ST_TYPE q : states) {
            stateMap[q] = i;
            i++;
        }

        construct_delta_ords(Qstars);
        // TODO: algorithm for selecting Ranges or Any
        construct_delta_anys(Qtilde);

        construct_delta_cs(Qstar_source, Qtilde);

        state2Qlabel[INV_STATE].kind = INV;
    }

    int codegen(const std::string &filename) {
        int fd = open(filename.c_str(), O_RDWR | O_TRUNC | O_CREAT, 0644);
        if (fd < 0) {
            perror("open");
            exit(1);
        }
        add_preprocess();
        add_tokenClass();
        add_vfaClass();

        int size = write(fd, code.c_str(), code.size());
        // int size = fwrite(code.c_str(), 1, code.size(), fp);
        close(fd);
        return 0;
    };
};
