#include <iostream>
#include <vector>
#include <fstream>
#include <random>
#include <sstream>
#include "Point.h"
#include "Cluster.h"
#include <chrono>

#define POINTS_NUMBER 2000000
#define CLUSTERS_NUMBER 15
#define MAX_RANGE 100000
#define MAX_ITERATION 100
#define PARALLEL 1

//-----------------------------WARNING----------------------------
//Activating animations will slow down computation regardless of
//choosing parallel or sequential version.
#define WITH_ANIMATION 0

int gnuplot_it = 0;

std::vector<Point> initPoints(int pointsNumber);

std::vector<Cluster> initCluster(int clustersNumber);

bool updateClusters(std::vector<Cluster> &clusters);

void kMeansIteration(std::vector<Point> &points, std::vector<Cluster> &clusters);

void kMeansIterationParallel(std::vector<Point> &points, std::vector<Cluster> &clusters);

void plot(std::vector<Point> &points);

int main() {
    bool convergence = false;
    int iterations = 0;
    unsigned long executionTime = 0;
    std::vector<Point> points;
    std::vector<Cluster> clusters;
    auto time = std::chrono::high_resolution_clock::now();
#pragma omp parallel
    {
#pragma omp sections
        {
#pragma omp section
            {
                points = initPoints(POINTS_NUMBER);
            }
#pragma omp section
            {
                clusters = initCluster(CLUSTERS_NUMBER);
            }
        }
    }

    system("mkdir images");

    while (!convergence && iterations < MAX_ITERATION) {
        iterations++;

        auto start = std::chrono::high_resolution_clock::now();

        if (PARALLEL) {
            kMeansIterationParallel(points, clusters);
        } else {
            kMeansIteration(points, clusters);
        }
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
        executionTime += duration.count();

        convergence = updateClusters(clusters);
        if (WITH_ANIMATION) {
            plot(points);
        }
        printf("Iteration %d completed \n", iterations);
    }

    plot(points);
    std::cout << executionTime << "ms" << std::endl;

    int delay = ceil(executionTime / (10 * gnuplot_it));  //calculating gif time

    std::string s = "convert -delay " + std::to_string(delay) + " images/xyz-*.png output_" +
                    (PARALLEL ? "parallel_" : "sequential_") + std::to_string(executionTime) + "ms.gif";
    system(s.c_str()); // creating gif
    system("rm -f images/*.png"); //removing images

    return 0;
}

std::vector<Point> initPoints(int pointsNumber) {
    std::vector<Point> points;
    points.reserve(pointsNumber);

    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(0, MAX_RANGE);

    for (int i = 0; i < pointsNumber; i++) {
        points.emplace_back(Point(distribution(generator), distribution(generator)));
    }

    return points;
}

std::vector<Cluster> initCluster(int clustersNumber) {
    std::vector<Cluster> clusters;
    clusters.reserve(clustersNumber);

    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(0, MAX_RANGE);

    for (int i = 0; i < clustersNumber; i++) {
        clusters.emplace_back(Cluster(distribution(generator), distribution(generator)));
    }

    return clusters;
}

double distance(const Point &point, const Cluster &cluster) {
    return sqrt(pow(point.getX() - cluster.getX(), 2) +
                pow(point.getY() - cluster.getY(), 2));
}

bool updateClusters(std::vector<Cluster> &clusters) {
    bool convergence = true;

    for (int i = 0; i < clusters.size(); i++) {
        if (clusters[i].update()) {
            convergence = false;
        }
        clusters[i].releasePoints();
    }

    return convergence;
}

void kMeansIterationParallel(std::vector<Point> &points, std::vector<Cluster> &clusters) {
    int clusterIndex;
    double minDist;

#pragma omp parallel default(shared) private(minDist, clusterIndex)
    {
#pragma omp for schedule(static)
        for (int i = 0; i < POINTS_NUMBER; i++) {
            Point &point = points[i];
            clusterIndex = 0;
            minDist = distance(point, clusters[0]);

            for (int j = 1; j < CLUSTERS_NUMBER; j++) {
                Cluster &cluster = clusters[j];
                double dist = distance(point, cluster);
                if (dist < minDist) {
                    minDist = dist;
                    clusterIndex = j;
                }
            }
            clusters[clusterIndex].addPoint(points[i]);
            points[i].setCluster(clusterIndex);
        }
    }
}

void kMeansIteration(std::vector<Point> &points, std::vector<Cluster> &clusters) {
    int clusterIndex;
    double minDist;

    for (int i = 0; i < POINTS_NUMBER; i++) {
        Point &point = points[i];
        clusterIndex = 0;
        minDist = distance(point, clusters[0]);

        for (int j = 1; j < CLUSTERS_NUMBER; j++) {
            Cluster &cluster = clusters[j];
            double dist = distance(point, cluster);
            if (dist < minDist) {
                minDist = dist;
                clusterIndex = j;
            }
        }
        clusters[clusterIndex].addPoint(points[i]);
        points[i].setCluster(clusterIndex);
    }

}

void plot(std::vector<Point> &points) {
    std::ofstream outfile("data.txt");
    gnuplot_it++;

    for (int i = 0; i < points.size(); i++) {
        Point point = points[i];
        outfile << point.getX() << " " << point.getY() << " " << point.getCluster() << std::endl;
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