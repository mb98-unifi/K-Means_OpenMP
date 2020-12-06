//
// Created by Mattia Bacci on 10/11/2020.
//

#ifndef K_MEANS_OPENMP_CLUSTER_H
#define K_MEANS_OPENMP_CLUSTER_H

#include <cmath>
#include "Point.h"

class Cluster {
public:
    Cluster() : Cluster(0, 0) {}

    Cluster(double x, double y) : x(x), y(y), tmpX(0), tmpY(0) {};

    void addPoint(const Point &point) {
#pragma omp atomic
        tmpX += point.getX();
#pragma omp atomic
        tmpY += point.getY();
#pragma omp atomic
        size++;
    }

    void releasePoints() {
        tmpX = 0;
        tmpY = 0;
        size = 0;
    }

    bool update() {
        if (std::abs(x - tmpX / size) < std::pow(10, -1) && std::abs(y - tmpY / size) < std::pow(10, -1)) {
            return false;
        }

        x = tmpX / size;
        y = tmpY / size;
        return true;
    }

    double getX() const {
        return x;
    }

    double getY() const {
        return y;
    }

private:
    double x, y, tmpX, tmpY;
    int size;
};

#endif //K_MEANS_OPENMP_CLUSTER_H
