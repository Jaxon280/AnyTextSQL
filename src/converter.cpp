#include "converter.hpp"

#include <iostream>

using namespace vlex;

std::vector<ST_TYPE> VectFA::construct_Qs() {
    std::vector<ST_TYPE> Qs;
    for (ST_TYPE q : states) {
        int next_c = 0;
        for (int j = 0; j < ASCII_SZ; j++) {
            if (transTable[q][j] != q) {
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
            if (transTable[qs][c] != qs) {
                q = transTable[qs][c];
                Qs.str += (char)c;
            }
        }
        Qs.states.insert(q);
        ST_TYPE qx = qs;

        while (true) {
            ST_TYPE qnext = 0, cnext = 0;
            int N = 0;
            for (int c = 0; c < ASCII_SZ; c++) {
                ST_TYPE qn = transTable[q][c];
                if (Qs.states.find(qn) != Qs.states.end() || qn == q) {
                    if (qn != transTable[qx][c]) goto end_dfs;
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
            qx = transTable[qx][cnext];
        }
    end_dfs:
        if (Qs.str.length() < ORD_LENGTH) continue;
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
                if (q == transTable[q][c]) {
                    Qtilde.insert(q);
                    break;
                }
            }
        }
    }
    return Qtilde;
}

void VectFA::construct_delta_ords(const std::vector<Qstar> &Qstar_set,
                                  std::map<ST_TYPE, int> opt_poses) {
    for (const Qstar &Qs : Qstar_set) {
        Delta *new_ord = new Delta;
        new_ord->startState = old2new[Qs.source];
        new_ord->str = Qs.str.substr(opt_poses[Qs.source]);
        new_ord->backStr = Qs.str.substr(0, opt_poses[Qs.source]);
        for (int i = 0; i <= 16; i++) {
            if (i <= 16 - new_ord->str.size()) {
                new_ord->rTable.push_back(old2new[Qs.sink]);
            } else {
                new_ord->rTable.push_back(old2new[Qs.source]);
            }
        }

        qlabels[old2new[Qs.source]].kind = ORDERED;
        qlabels[old2new[Qs.source]].delta = new_ord;
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
            new_trans->startState = old2new[q];
            new_trans->charTable.resize(ASCII_SZ);

            std::vector<int> chars;
            for (int c = 0; c < ASCII_SZ; c++) {
                new_trans->charTable[c] = old2new[transTable[q][c]];
                if (transTable[q][c] != q) {
                    chars.push_back(c);
                }
            }

            if (chars.size() > 16) {
                if (construct_delta_ranges(new_trans, chars)) {
                    qlabels[old2new[q]].delta = new_trans;
                    qlabels[old2new[q]].kind = RANGES;
                }
            } else {
                for (int c : chars) {
                    new_trans->str += (char)c;
                    qlabels[old2new[q]].delta = new_trans;
                    qlabels[old2new[q]].kind = ANY;
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
            new_c->startState = old2new[q];
            new_c->charTable.resize(ASCII_SZ);
            for (int c = 0; c < ASCII_SZ; c++) {
                new_c->charTable[c] = old2new[transTable[q][c]];
            }

            qlabels[old2new[q]].delta = new_c;
            qlabels[old2new[q]].kind = C;
        }
    }
}

VectFA::VectFA(DFA &_dfa) { transTable = _dfa.getTransTable(); }

VectFA::VectFA(DFA &_dfa, DATA_TYPE *_data, SIZE_TYPE _size) {
    transTable = _dfa.getTransTable();
    constructVFA(_dfa, _data, _size);
}

void VectFA::constructVFA(DFA &dfa, DATA_TYPE *data, SIZE_TYPE size) {
    for (ST_TYPE i = INV_STATE; i < dfa.getNumStates(); i++) {
        states.insert(i);
    }
    for (ST_TYPE s : dfa.getAcceptStates()) {
        acceptStates.insert(s);
    }

    // todo: add checking access to state
    // std::map<ST_TYPE, std::set<ST_TYPE>> Q_access;
    // for (ST_TYPE i : states) {
    //     for (int j = 0; j < ASCII_SZ; j++) {
    //         Q_access[dfa[i][j]].insert(i);
    //     }
    // }

    std::vector<ST_TYPE> Qs = construct_Qs();
    std::vector<Qstar> Qstars = construct_Qstars(Qs);

    vlex::PFA pfa(transTable, dfa.getNumStates(), data, size, 0);
    pfa.scan_ords(Qstars);
    std::map<ST_TYPE, int> opt_poses = pfa.calc_ords();

    pfa.scan();
    pfa.calc();

    std::set<ST_TYPE> Qstar_source;
    for (Qstar Qst : Qstars) {
        Qstar_source.insert(Qst.source);
        for (ST_TYPE q : Qst.states) {
            if (q != Qst.source && q != Qst.sink) {
                states.erase(q);
            }
        }
    }

    std::set<ST_TYPE> Qtilde = construct_Qtilde(Qstar_source);

    int i = 0;
    for (ST_TYPE q : states) {
        Qlabel qlabel;
        if (acceptStates.find(q) != acceptStates.end()) {
            qlabel.isAccept = true;
        }
        qlabels.push_back(qlabel);
        old2new[q] = i;
        i++;
    }

    construct_delta_ords(Qstars, opt_poses);
    construct_delta_anys(Qtilde, pfa);
    construct_delta_cs(Qstar_source, Qtilde);
    qlabels[INV_STATE].kind = INV;
}

int VectFA::codegen(const std::string &filename) {
    Codegen cgen(filename, qlabels, states);
    cgen.generate();
    return 0;
}
