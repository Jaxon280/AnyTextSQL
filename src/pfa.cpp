#include "scanner/pfa.hpp"

namespace vlex {

PFA::PFA(std::vector<std::vector<ST_TYPE>> &_transTable, int _numStates,
         DATA_TYPE *_data, SIZE_TYPE _size, SIZE_TYPE _start)
    : transTable(_transTable),
      numStates(_numStates),
      data(_data),
      size(_size),
      start(_start) {
    countMat = (int **)malloc(sizeof(int *) * numStates);
    for (int j = 0; j < numStates; j++) {
        countMat[j] = (int *)malloc(sizeof(int) * numStates);
        for (int k = 0; k < numStates; k++) {
            countMat[j][k] = 0;
        }
    }

    sumCursVec = (int *)malloc(sizeof(int) * numStates);
    for (int j = 0; j < numStates; j++) {
        sumCursVec[j] = 0;
    }
    sumNextsVec = (int *)malloc(sizeof(int) * numStates);
    for (int j = 0; j < numStates; j++) {
        sumNextsVec[j] = 0;
    }
}

void PFA::scanSubString(const std::vector<vlex::Qstar> &Qstars) {
    SIZE_TYPE i = start;

    for (vlex::Qstar Qs : Qstars) {
        patterns.insert(std::pair<ST_TYPE, std::string>(Qs.source, Qs.str));
        std::vector<std::vector<int>> counts = {{0, 0}, {0, 0}, {0, 0}, {0, 0}};
        countOrds.insert(std::pair<ST_TYPE, std::vector<std::vector<int>>>(
            Qs.source, counts));
    }

    for (auto &[s, countOrd] : countOrds) {
        std::string p = patterns[s];
        i = start;
        while (i < size && data[i]) {
            if (data[i] == p[0]) {
                countOrd[0][0]++;
                if (data[i + 1] == p[1]) countOrd[0][1]++;
            } else if (data[i] == p[1]) {
                countOrd[1][0]++;
                if (data[i + 1] == p[2]) countOrd[1][1]++;
            } else if (data[i] == p[2]) {
                countOrd[2][0]++;
                if (data[i + 1] == p[3]) countOrd[2][1]++;
            } else if (data[i] == p[3]) {
                countOrd[3][0]++;
                if (data[i + 1] == p[4]) countOrd[3][1]++;
            }
            i++;
        }
    }

    sumAll = i - start;
}

std::map<ST_TYPE, int> PFA::calcSubString() const {
    long long min = 10000000000;
    std::map<ST_TYPE, int> argmins;
    for (const auto &[s, countOrd] : countOrds) {
        int argmin = 0;
        argmins.insert(std::pair<ST_TYPE, int>(s, argmin));
        for (int j = 0; j < 4; j++) {
            double prob0 = (double)(countOrd[j][0]) / (double)sumAll;
            printf("pattern %d/4, probability[1] %f\n", j + 1, prob0);
            double prob1 = (double)(countOrd[j][1]) / (double)sumAll;
            printf("pattern %d/4, probability[2] %f\n", j + 1, prob1);
            double prob =
                (double)(countOrd[j][0] + countOrd[j][1]) / (double)sumAll;
            printf("pattern %d/4, probability sum %f\n", j + 1, prob);

            if (countOrd[j][0] + countOrd[j][1] < min) {
                min = countOrd[j][0] + countOrd[j][1];
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
        countMat[curS][nextS]++;
        sumCursVec[curS]++;
        sumNextsVec[nextS]++;

        if (currentState == INV_STATE) {
            currentState = INIT_STATE;
        }
        i++;
    }
}

double PFA::calc(ST_TYPE j, ST_TYPE k) const {
    if (sumCursVec[j] != 0)
        return (double)countMat[j][k] / (double)sumCursVec[j];
    else
        return 0.0;
}

void PFA::calc() const {
    for (int j = 0; j < numStates; j++) {
        for (int k = 0; k < numStates; k++) {
            if (sumCursVec[j] == 0) {
                continue;
            }
            double prob = (double)countMat[j][k] / (double)sumCursVec[j];
            if (prob > 0.0) {
                printf("q%d -> q%d 's probability: %f\n", j, k, prob);
            }
        }
    }
}

}  // namespace vlex
