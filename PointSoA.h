//
// Created by Mattia Bacci on 22/01/21.
//

#ifndef K_MEANS_OPENMP_POINTSOA_H
#define K_MEANS_OPENMP_POINTSOA_H

class PointSoA {
public:

    PointSoA(int n) {
        x = new double[n];
        y = new double[n];
        cluster = new int[n];

        for (int i = 0; i < n; i++) {
            x[i] = 0;
            y[i] = 0;
            cluster[i] = 0;
        }
    }

    double* x;
    double* y;
    int* cluster;
};
#endif //K_MEANS_OPENMP_POINTSOA_H
