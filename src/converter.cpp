#include "../include/converter.hpp"

#include "../include/codegen.hpp"
#include "../include/common.hpp"
#include "../include/interface.hpp"

using namespace vlex;

std::vector<ST_TYPE> VectFA::construct_Qs(int state_sz) {
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

std::vector<Qstar> VectFA::construct_Qstars(std::vector<ST_TYPE> Qsource) {
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

std::set<ST_TYPE> VectFA::construct_Qtilde(std::set<ST_TYPE> Qstar_source) {
    std::set<ST_TYPE> Qtilde;

    for (ST_TYPE q : states) {
        int count = 0;
        for (int c = 0; c < ASCII_SZ; c++) {
            if (q == dfa[q][c]) {
                count++;
            }
        }

        if (q != INV_STATE && Qstar_source.find(q) == Qstar_source.end() &&
            count > 1) {
            Qtilde.insert(q);
        }
    }
    return Qtilde;
}

void VectFA::construct_delta_ords(std::vector<Qstar> Qstar_set) {
    for (Qstar Qs : Qstar_set) {
        Delta *new_ord = new Delta;
        new_ord->startState = Qs.source;
        new_ord->str = Qs.str;
        for (int i = 0; i <= 16; i++) {
            if (i <= 16 - new_ord->str.size()) {
                new_ord->r_table.push_back(Qs.sink);
            } else {
                new_ord->r_table.push_back(Qs.source);
            }
        }

        qlabels[Qs.source].kind = ORDERED;
        qlabels[Qs.source].delta = new_ord;
        qlabels[Qs.source].is_r = true;
        qlabels[Qs.sink].is_sink = true;
        qlabels[Qs.sink].c_length = new_ord->str.size();
    }
}

void VectFA::construct_delta_anys(std::set<ST_TYPE> Qtilde) {
    for (ST_TYPE q : Qtilde) {
        Delta *new_any = new Delta;
        new_any->startState = q;
        new_any->char_table.resize(ASCII_SZ);

        qlabels[q].delta = new_any;
        qlabels[q].kind = ANY;
        qlabels[q].is_r = true;
        for (int c = 0; c < ASCII_SZ; c++) {
            new_any->char_table[c] = dfa[q][c];
            if (dfa[q][c] != q) {
                new_any->str += (char)c;
                qlabels[dfa[q][c]].is_inc = true;
            }
        }
    }
}

void VectFA::construct_delta_cs(std::set<ST_TYPE> Qstar_source,
                                std::set<ST_TYPE> Qtilde) {
    for (ST_TYPE q : states) {
        if (Qstar_source.find(q) == Qstar_source.end() &&
            Qtilde.find(q) == Qtilde.end()) {
            Delta *new_c = new Delta;
            new_c->startState = q;
            new_c->char_table.resize(ASCII_SZ);

            qlabels[q].delta = new_c;
            qlabels[q].kind = C;
            for (int c = 0; c < ASCII_SZ; c++) {
                new_c->char_table[c] = dfa[q][c];
                qlabels[dfa[q][c]].is_inc = true;
            }
        }
    }
}

VectFA::VectFA(ST_TYPE **fa, ST_TYPE *accepts, int stateSize,
               int acceptStateSize) {
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
        Qlabel qlabel;
        qlabel.state = i;
        if (acceptStates.find(q) != acceptStates.end()) {
            qlabel.is_accept = true;
        }
        qlabels[q] = qlabel;

        i++;
    }

    construct_delta_ords(Qstars);
    // TODO: algorithm for selecting Ranges or Any
    construct_delta_anys(Qtilde);

    construct_delta_cs(Qstar_source, Qtilde);

    qlabels[INV_STATE].kind = INV;
}

int VectFA::codegen(const std::string &filename) {
    Codegen cgen(filename, qlabels, states);
    cgen.generate();
    return 0;
}
