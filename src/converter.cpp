#include "converter.hpp"

#include <iostream>

using namespace vlex;

std::vector<ST_TYPE> VectFA::construct_Qs() {
    std::vector<ST_TYPE> Qs;
    for (ST_TYPE q : states) {
        // if (q == INV_STATE) continue;
        if (q != INIT_STATE) {
            continue;
        }  // TODO: able to handle non-INIT State
        int next_c = 0;
        for (int j = 0; j < ASCII_SZ; j++) {
            if (dfa[q][j] != q) {
                next_c++;
                if (next_c > 1) break;
            }
        }
        if (next_c == 1) Qs.push_back(q);
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

std::set<ST_TYPE> VectFA::construct_Qtilde(
    const std::set<ST_TYPE> &Qstar_source) {
    std::set<ST_TYPE> Qtilde;

    for (ST_TYPE q : states) {
        if (q != INV_STATE && Qstar_source.find(q) == Qstar_source.end()) {
            for (int c = 0; c < ASCII_SZ; c++) {
                if (q == dfa[q][c]) {
                    Qtilde.insert(q);
                    break;
                }
            }
        }
    }
    return Qtilde;
}

void VectFA::construct_delta_ords(const std::vector<Qstar> &Qstar_set,
                                  int opt_pos) {
    for (const Qstar &Qs : Qstar_set) {
        Delta *new_ord = new Delta;
        new_ord->startState = Qs.source;
        if (Qs.source == INIT_STATE) {
            new_ord->str = Qs.str.substr(opt_pos);
            new_ord->back_str = Qs.str.substr(0, opt_pos);
        } else {
            new_ord->str = Qs.str;
        }
        for (int i = 0; i <= 16; i++) {
            if (i <= 16 - new_ord->str.size()) {
                new_ord->r_table.push_back(Qs.sink);
            } else {
                new_ord->r_table.push_back(Qs.source);
            }
        }

        qlabels[Qs.source].kind = ORDERED;
        qlabels[Qs.source].delta = new_ord;
    }
}

int VectFA::construct_delta_ranges(Delta *trans,
                                   const std::vector<int> &chars) {
    std::vector<std::vector<int>> ranges;
    for (int i = 0; i < chars.size(); i++) {
        std::vector<int> range;
        range.push_back(chars[i]);
        while (i < chars.size() - 1 && chars[i + 1] - chars[i] == 1) {
            range.push_back(chars[i + 1]);
            i++;
        }

        ranges.push_back(range);
    }

    if (ranges.size() <= 8) {
        int start, end;
        for (std::vector<int> range : ranges) {
            start = range[0];
            end = range[range.size() - 1];
            trans->str.push_back(start);
            trans->str.push_back(end);
        }
        return 1;
    } else {
        return 0;
    }
}

void VectFA::construct_delta_anys(std::set<ST_TYPE> &Qtilde, const PFA &pfa) {
    const double p = 0.75;
    for (auto it = Qtilde.begin(); it != Qtilde.end();) {
        ST_TYPE q = *it;
        if (pfa.calc(q, q) < p) {
            it = Qtilde.erase(it);
        } else {
            Delta *new_trans = new Delta;
            new_trans->startState = q;
            new_trans->char_table.resize(ASCII_SZ);

            std::vector<int> chars;
            for (int c = 0; c < ASCII_SZ; c++) {
                new_trans->char_table[c] = dfa[q][c];
                if (dfa[q][c] != q) {
                    chars.push_back(c);
                }
            }

            if (chars.size() > 16) {
                if (construct_delta_ranges(new_trans, chars)) {
                    qlabels[q].delta = new_trans;
                    qlabels[q].kind = RANGES;
                }
            } else {
                for (int c : chars) {
                    new_trans->str += (char)c;
                    qlabels[q].delta = new_trans;
                    qlabels[q].kind = ANY;
                }
            }
            ++it;
        }
    }
}

void VectFA::construct_delta_cs(const std::set<ST_TYPE> &Qstar_source,
                                const std::set<ST_TYPE> &Qtilde) {
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
            }
        }
    }
}

VectFA::VectFA(ST_TYPE **fa, ST_TYPE *accepts, int stateSize,
               int acceptStateSize, char *data, int size) {
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

    vlex::PFA pfa(fa, stateSize, data, size, 0);

    std::vector<ST_TYPE> Qs = construct_Qs();
    std::vector<Qstar> Qstars = construct_Qstars(Qs);

    // construct PFA
    pfa.construct_ordPFA(Qstars, 4, 3);
    pfa.scan_ord(0.002);
    int opt_pos = pfa.calc_ord();

    pfa.scan(0.002);
    // pfa.calc();

    std::set<ST_TYPE> Qstar_source;
    for (Qstar Qst : Qstars) {
        Qstar_source.insert(Qst.source);
        for (ST_TYPE q : Qst.states) {
            if (q != Qst.source && q != Qst.sink) {
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

    construct_delta_ords(Qstars, opt_pos);
    construct_delta_anys(Qtilde, pfa);

    construct_delta_cs(Qstar_source, Qtilde);

    qlabels[INV_STATE].kind = INV;
}

int VectFA::codegen(const std::string &filename) {
    Codegen cgen(filename, qlabels, states);
    cgen.generate();
    return 0;
}
