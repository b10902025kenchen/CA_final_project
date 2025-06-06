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
    vector<Point2<double>> prev_gradients;
    vector<Point2<double>> prev_positions;
    vector<Point2<double>> step_sizes;
    vector<Point2<double>> step_sizes_bound; // Gradients for each machine
    BinGrid bin_grid;
    Optimizer(double x0, double x1, double y0, double y1, vector<Star*> stars);
    Optimizer() {}
    ~Optimizer() { cout << "Optimizer destroyed" << endl; }
    double constraint_fac = 1;
    double density_fac = 1;
    double maximum_achieve_d_gradient = 0; 
    double maximum_achieve_c_gradient = 0;
    double max_charge_density = 1;
    bool use_constraint = true;
    bool use_density = true;
    int bound_x;
    int bound_y;
    void density_init();
    void updateGradients();
    void updatePositions();
    void normalize_step_sizes();
    void step_size_prediction();
    void update_constraint_fac() { constraint_fac *= 1.01; constraint_fac = min(constraint_fac, 10000.0); } // Limit the constraint factor to prevent overflow
    void setUseConstraint(bool use) { use_constraint = use; }
    void setUseDensity(bool use) { use_density = use; }
    void set_step_size_bound(double step_size_x, double step_size_y) {
        for (int i = 0; i < stars.size(); ++i) {
            step_sizes_bound[i] = Point2<double>(step_size_x, step_size_y);
        }
    }
    void operator()();
    vector<Point2<double>> getGradients() const { return gradients; }
    vector<Point2<double>> constraintGradients() ;
};
bool inside_interval(const Star& star) ;

#endif
