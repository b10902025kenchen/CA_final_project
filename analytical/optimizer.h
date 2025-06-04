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
    bool use_constraint = true;
    int bound_x;
    int bound_y;
    void updateGradients();
    void updatePositions();
    void normalize_step_sizes();
    void update_constraint_fac() { constraint_fac *= 1.01; constraint_fac = min(constraint_fac, 10000.0); } // Limit the constraint factor to prevent overflow
    void setUseConstraint(bool use) { use_constraint = use; }
    void operator()();
    vector<Point2<double>> getGradients() const { return gradients; }
    vector<Point2<double>> constraintGradients() ;
};
bool inside_interval(const Star& star) ;

#endif
