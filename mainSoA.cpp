//
// Created by Mattia Bacci on 22/01/21.
//

#include <iostream>
#include <vector>
#include <fstream>
#include <random>
#include <sstream>
#include "PointSoA.h"
#include "ClusterSoA.h"
#include <chrono>

#define POINTS_NUMBER 100000
#define CLUSTERS_NUMBER 10
#define MAX_RANGE 100000
#define MAX_ITERATION 100
#define PARALLEL 1

//-----------------------------WARNING----------------------------
//Activating animations will slow down computation regardless of
//choosing parallel or sequential version.
#define WITH_ANIMATION 0

int gnuplot_it = 0;

PointSoA initPoints();

ClusterSoA initCluster();

bool updateClusters(ClusterSoA clusterSoA);

void kMeansIteration(PointSoA pointSoA, ClusterSoA clusterSoA);

void kMeansIterationParallel(PointSoA pointSoA, ClusterSoA clusterSoA);

void assignPoints(int i, PointSoA pointSoA, ClusterSoA clusterSoA);

void plot(PointSoA pointSoA);

int main() {
    bool convergence = false;
    int iterations = 0;
    unsigned long executionTime = 0;
    PointSoA pointSoA = initPoints();
    ClusterSoA clusterSoA = initCluster();
    auto time = std::chrono::high_resolution_clock::now();

    system("mkdir images");

    while (!convergence && iterations < MAX_ITERATION) {
        iterations++;

        auto start = std::chrono::high_resolution_clock::now();

        if (PARALLEL) {
            kMeansIterationParallel(pointSoA, clusterSoA);
        } else {
            kMeansIteration(pointSoA, clusterSoA);
        }
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
        executionTime += duration.count();

        convergence = updateClusters(clusterSoA);
        if (WITH_ANIMATION) {
            plot(pointSoA);
        }
        printf("Iteration %d completed \n", iterations);
    }

    plot(pointSoA);
    std::cout << executionTime << "ms" << std::endl;

    int delay = ceil(executionTime / (10 * gnuplot_it));  //calculating gif time

    std::string s = "convert -delay " + std::to_string(delay) + " images/xyz-*.png output_" +
                    (PARALLEL ? "parallel_" : "sequential_") + std::to_string(executionTime) + "ms.gif";
    system(s.c_str()); // creating gif
    system("rm -f images/*.png"); //removing images

    return 0;
}

PointSoA initPoints() {
    PointSoA pointSoA(POINTS_NUMBER);

    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(0, MAX_RANGE);

    for (int i = 0; i < POINTS_NUMBER; i++) {
        pointSoA.x[i] = distribution(generator);
        pointSoA.y[i] = distribution(generator);
    }

    return pointSoA;
}

ClusterSoA initCluster() {
    ClusterSoA clusterSoA(CLUSTERS_NUMBER);

    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(0, MAX_RANGE);

    for (int i = 0; i < CLUSTERS_NUMBER; i++) {
        clusterSoA.x[i] = distribution(generator);
        clusterSoA.y[i] = distribution(generator);
    }

    return clusterSoA;
}

double distance(double pointX, double pointY, double clusterX, double clusterY) {
    return sqrt(pow(pointX - clusterX, 2) +
                pow(pointY - clusterY, 2));
}

bool updateClusters(ClusterSoA clusterSoA) {
    bool convergence = true;

    for (int i = 0; i < CLUSTERS_NUMBER; i++) {
        if (clusterSoA.update(i)) {
            convergence = false;
        }
        clusterSoA.releasePoints(i);
    }

    return convergence;
}


void kMeansIterationParallel(PointSoA pointSoA, ClusterSoA clusterSoA) {

#pragma omp parallel default(shared)
    {
#pragma omp for schedule(static)
        for (int i = 0; i < POINTS_NUMBER; i++) {
            assignPoints(i, pointSoA, clusterSoA);
        }
    }
}

void kMeansIteration(PointSoA pointSoA, ClusterSoA clusterSoA) {
    for (int i = 0; i < POINTS_NUMBER; i++) {
        assignPoints(i, pointSoA, clusterSoA);
    }
}

void assignPoints(int i, PointSoA pointSoA, ClusterSoA clusterSoA){
    int clusterIndex = 0;
    double minDist = distance(pointSoA.x[i], pointSoA.y[i], clusterSoA.x[0], clusterSoA.y[0]);

    for (int j = 1; j < CLUSTERS_NUMBER; j++) {
        double dist = distance(pointSoA.x[i], pointSoA.y[i], clusterSoA.x[j], clusterSoA.y[j]);
        if (dist < minDist) {
            minDist = dist;
            clusterIndex = j;
        }
    }
    clusterSoA.addPoint(clusterIndex, pointSoA.x[i], pointSoA.y[i]);
    pointSoA.cluster[i] = clusterIndex;
}

void plot(PointSoA pointSoA) {
    std::ofstream outfile("data.txt");
    gnuplot_it++;

    for (int i = 0; i < POINTS_NUMBER; i++) {
        outfile << pointSoA.x[i] << " " << pointSoA.y[i] << " " << pointSoA.cluster[i] << std::endl;
    }

    std::stringstream ss;
    ss << std::setw(3) << std::setfill('0') << gnuplot_it;
    std::string s = ss.str();

    outfile.close();
    std::string gnuplot = "gnuplot -p -e \"set terminal png size 800,700; set output 'images/xyz-" + s +
                          ".png'; plot 'data.txt' using 1:2:3 with points palette notitle\"";
    system(gnuplot.c_str());
    remove("data.txt");
}