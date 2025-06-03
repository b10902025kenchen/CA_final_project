#ifndef _OPTIMIZER_H_
#define _OPTIMIZER_H_

#include <iostream>
#include <vector>
#include "Point.h"
#include "Star.h"
#include "BinGrid.h"

using namespace std;

class Optimizer {

    public:
    vector<Star*> stars;
    vector<Point2<double>> gradients;
    vector<double> step_sizes;
    BinGrid bin_grid;
    Optimizer(double x0, double x1, double y0, double y1, vector<Star*> stars);
    Optimizer() {}
    ~Optimizer() { cout << "Optimizer destroyed" << endl; }
    double constraint_fac = 1;
    int bound_x;
    int bound_y;
    void updateGradients();
    void updatePositions();
    void operator()();
    vector<Point2<double>> getGradients() const { return gradients; }
    vector<Point2<double>> constraintGradients() ;
};


#endif
