#include "pfa.hpp"

using namespace vlex;

PFA::PFA(ST_TYPE **dfa, int numStates, char *data, int size, int start)
    : dfa(dfa), numStates(numStates), data(data), size(size), i(start) {
    count_mat = (int **)malloc(sizeof(int *) * numStates);
    for (int j = 0; j < numStates; j++) {
        count_mat[j] = (int *)malloc(sizeof(int) * numStates);
        for (int k = 0; k < numStates; k++) {
            count_mat[j][k] = 0;
        }
    }

    sum_curs_vec = (int *)malloc(sizeof(int) * numStates);
    for (int j = 0; j < numStates; j++) {
        sum_curs_vec[j] = 0;
    }
    sum_nexts_vec = (int *)malloc(sizeof(int) * numStates);
    for (int j = 0; j < numStates; j++) {
        sum_nexts_vec[j] = 0;
    }
}

void PFA::construct_ordPFA(std::vector<vlex::Qstar> Qstars, int n_patterns0,
                           int length) {
    for (vlex::Qstar Qs : Qstars) {
        if (Qs.source == INIT_STATE) {
            pattern += Qs.str;
            if (length < 1) {
                length = 1;
            }
            if ((n_patterns0 + length - 1) > pattern.size()) {
                n_patterns = pattern.size() / 2;
                length = pattern.size() / 2;
            } else {
                n_patterns = n_patterns0;
            }
        }
    }
    n_states = length + 2;

    ord_dfa = (ST_TYPE ***)malloc(sizeof(ST_TYPE **) * n_patterns);
    for (int j = 0; j < n_patterns; j++) {
        ord_dfa[j] = (ST_TYPE **)malloc(sizeof(ST_TYPE *) * n_states);
        int X = INIT_STATE;
        std::string pat = pattern.substr(j, 4);
        ord_dfa[j][0] = (ST_TYPE *)malloc(sizeof(ST_TYPE) * ASCII_SZ);
        for (int l = 0; l < ASCII_SZ; l++) {
            ord_dfa[j][0][l] = INIT_STATE;
        }
        ord_dfa[j][1] = (ST_TYPE *)malloc(sizeof(ST_TYPE) * ASCII_SZ);
        for (int l = 0; l < ASCII_SZ; l++) {
            ord_dfa[j][1][l] = INIT_STATE;
        }
        ord_dfa[j][INIT_STATE][(int)pat[0]] = 2;

        for (int k = 2; k < n_states; k++) {
            ord_dfa[j][k] = (ST_TYPE *)malloc(sizeof(ST_TYPE) * ASCII_SZ);
            for (int l = 0; l < ASCII_SZ; l++) {
                ord_dfa[j][k][l] = ord_dfa[j][X][l];
            }

            if (k != n_states - 1) {
                ord_dfa[j][k][(int)pat[k - 1]] = k + 1;
                X = ord_dfa[j][X][(int)pat[k - 1]];
            }
        }
    }

    count_ord = (int ***)malloc(sizeof(int **) * n_patterns);
    for (int j = 0; j < n_patterns; j++) {
        count_ord[j] = (int **)malloc(sizeof(int *) * n_states);
        for (int k = 0; k < n_states; k++) {
            count_ord[j][k] = (int *)malloc(sizeof(int) * n_states);
            for (int l = 0; l < n_states; l++) {
                count_ord[j][k][l] = 0;
            }
        }
    }

    sum_curs_ord = (int **)malloc(sizeof(int *) * n_patterns);
    for (int j = 0; j < n_patterns; j++) {
        sum_curs_ord[j] = (int *)malloc(sizeof(int) * n_states);
        for (int k = 0; k < n_states; k++) {
            sum_curs_ord[j][k] = 0;
        }
    }
    sum_nexts_ord = (int **)malloc(sizeof(int *) * n_patterns);
    for (int j = 0; j < n_patterns; j++) {
        sum_nexts_ord[j] = (int *)malloc(sizeof(int) * n_states);
        for (int k = 0; k < n_states; k++) {
            sum_nexts_ord[j][k] = 0;
        }
    }

    currentStates = (int *)malloc(sizeof(int) * n_patterns);
    for (int j = 0; j < n_patterns; j++) {
        currentStates[j] = INIT_STATE;
    }
}

void PFA::scan_ord(double lr) {
    int lsize = (int)size * lr + i;
    int start = i;
    int curS, nextS;
    while (i < lsize && data[i]) {
        for (int j = 0; j < n_patterns; j++) {
            curS = currentStates[j];
            currentStates[j] = ord_dfa[j][currentStates[j]][data[i]];
            nextS = currentStates[j];
            count_ord[j][curS][nextS]++;
            sum_curs_ord[j][curS]++;
            sum_nexts_ord[j][nextS]++;
        }
        i++;
    }
    sum_all = i - start;
}

int PFA::calc_ord() const {
    const double ord_latency = 20.0;
    const double cmp_latency = 1.0;

    double min = 1000000.0;
    int argmin = 0;
    for (int j = 0; j < n_patterns; j++) {
        double prob = 0.0;
        double score = 0.0;
        for (int k = 2; k < n_states; k++) {
            prob += (double)sum_nexts_ord[j][k] / (double)sum_all;
            // printf(
            //     "pattern x_{i+%d} ... x_{i+16} = z_{%d} ... z_{%d} 's "
            //     "probability: %f\n",
            //     18 - k, j + 1, j + k - 1, prob);
        }

        score = prob * (ord_latency + j * cmp_latency);
        if (score < min) {
            min = score;
            argmin = j;
        }
    }

    return argmin;
}

void PFA::scan(double lr) {
    int lsize = (int)size * lr + i;
    int curS, nextS;
    while (i < lsize && data[i]) {
        curS = currentState;
        currentState = dfa[currentState][data[i]];
        nextS = currentState;
        count_mat[curS][nextS]++;
        sum_curs_vec[curS]++;
        sum_nexts_vec[nextS]++;

        if (currentState == INV_STATE) {
            currentState = INIT_STATE;
        }
        i++;
    }
}

double PFA::calc(ST_TYPE j, ST_TYPE k) const {
    if (sum_curs_vec[j] != 0)
        return (double)count_mat[j][k] / (double)sum_curs_vec[j];
    else
        return 0.0;
}

void PFA::calc() const {
    for (int j = 0; j < numStates; j++) {
        for (int k = 0; k < numStates; k++) {
            if (sum_curs_vec[j] == 0) {
                continue;
            }
            double prob = (double)count_mat[j][k] / (double)sum_curs_vec[j];
            if (prob > 0.0) {
                printf("q%d -> q%d 's probability: %f\n", j, k, prob);
            }
        }
    }
}
