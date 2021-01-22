//
// Created by Mattia Bacci on 22/01/21.
//

#ifndef K_MEANS_OPENMP_CLUSTERSOA_H
#define K_MEANS_OPENMP_CLUSTERSOA_H

#include <cmath>

class ClusterSoA {
public:
    ClusterSoA(int n) {
         x = new double[n];
         y = new double[n];
         tmpX = new double[n];
         tmpY = new double[n];
         size = new int[n];

        for (int i = 0; i < n; i++) {
            x[i] = 0;
            y[i] = 0;
            tmpX[i] = 0;
            tmpY[i] = 0;
            size[i] = 0;
        }
    }


    void addPoint(int i, double pointX, double pointY) {
#pragma omp atomic
        tmpX[i] += pointX;
#pragma omp atomic
        tmpY[i] += pointY;
#pragma omp atomic
        size[i]++;
    }

    void releasePoints(int i) {
        tmpX[i] = 0;
        tmpY[i] = 0;
        size[i] = 0;
    }

    bool update(int i) {
        if (std::abs(x[i] - tmpX[i] / size[i]) < std::pow(10, -1) && std::abs(y[i] - tmpY[i] / size[i]) < std::pow(10, -1)) {
            return false;
        }

        x[i] = tmpX[i] / size[i];
        y[i] = tmpY[i] / size[i];
        return true;
    }

    double* x, *y, *tmpX, *tmpY;
    int *size;
};
#endif //K_MEANS_OPENMP_CLUSTERSOA_H
