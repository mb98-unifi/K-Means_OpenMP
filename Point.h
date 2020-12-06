//
// Created by Mattia Bacci on 10/11/2020.
//

#ifndef K_MEANS_OPENMP_POINT_H
#define K_MEANS_OPENMP_POINT_H

class Point {
public:

    Point() : Point(0, 0) {}

    Point(double x, double y) : x(x), y(y), cluster(0) {}


    double getX() const {
        return x;
    }

    double getY() const {
        return y;
    }

    int getCluster() const {
        return cluster;
    }

    void setCluster(int cluster) {
        Point::cluster = cluster;
    }

private:
    double x, y;
    int cluster;
};

#endif //K_MEANS_OPENMP_POINT_H
