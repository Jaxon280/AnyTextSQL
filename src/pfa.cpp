#include "scanner/pfa.hpp"

using namespace vlex;

PFA::PFA(std::vector<std::vector<ST_TYPE>> &_transTable, int _numStates,
         DATA_TYPE *_data, SIZE_TYPE _size, SIZE_TYPE _start)
    : transTable(_transTable),
      numStates(_numStates),
      data(_data),
      size(_size),
      start(_start) {
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

void PFA::scan_ords(std::vector<vlex::Qstar> Qstars) {
    SIZE_TYPE i = start;

    for (vlex::Qstar Qs : Qstars) {
        patterns.insert(std::pair<ST_TYPE, std::string>(Qs.source, Qs.str));
        std::vector<std::vector<int>> counts = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};
        count_ords.insert(std::pair<ST_TYPE, std::vector<std::vector<int>>>(
            Qs.source, counts));
    }

    for (auto &[s, count_ord] : count_ords) {
        std::string p = patterns[s];
        i = start;
        while (i < size && data[i]) {
            if (data[i] == p[0]) {
                count_ord[0][0]++;
                if (data[i + 1] == p[1]) count_ord[0][1]++;
            } else if (data[i] == p[1]) {
                count_ord[1][0]++;
                if (data[i + 1] == p[2]) count_ord[1][1]++;
            } else if (data[i] == p[2]) {
                count_ord[2][0]++;
                if (data[i + 1] == p[3]) count_ord[2][1]++;
            } else if (data[i] == p[3]) {
                count_ord[3][0]++;
                if (data[i + 1] == p[4]) count_ord[3][1]++;
            }
            i++;
        }
    }

    sum_all = i - start;
}

std::map<ST_TYPE, int> PFA::calc_ords() const {
    long long min = 10000000000;
    std::map<ST_TYPE, int> argmins;
    for (const auto &[s, count_ord] : count_ords) {
        int argmin = 0;
        argmins.insert(std::pair<ST_TYPE, int>(s, argmin));
        for (int j = 0; j < 4; j++) {
            double prob0 = (double)(count_ord[j][0]) / (double)sum_all;
            printf("pattern %d/4, probability[1] %f\n", j + 1, prob0);
            double prob1 = (double)(count_ord[j][1]) / (double)sum_all;
            printf("pattern %d/4, probability[2] %f\n", j + 1, prob1);
            double prob =
                (double)(count_ord[j][0] + count_ord[j][1]) / (double)sum_all;
            printf("pattern %d/4, probability sum %f\n", j + 1, prob);

            if (count_ord[j][0] + count_ord[j][1] < min) {
                min = count_ord[j][0] + count_ord[j][1];
                argmins[s] = j;
            }
        }
    }

    return argmins;
}

void PFA::scan() {
    int curS, nextS;
    SIZE_TYPE i = start;
    while (i < size && data[i]) {
        curS = currentState;
        currentState = transTable[currentState][data[i]];
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
