#ifndef _OPTIMIZER_H_
#define _OPTIMIZER_H_

#include <iostream>
#include <vector>
#include "Point.h"
#include "Star.h"
#include "BinGrid.h"
#include "placement_base.h"

using namespace std;

class Optimizer {

    public:
    Star_Placement& placement;
    vector<Star*> stars;
    vector<Point2<double>> gradients;
    vector<double> step_sizes;
    BinGrid bin_grid;
    Optimizer(Star_Placement& placement);
    void updateGradients();
    void updatePositions();
    void operator()();
    vector<Point2<double>> getGradients() const { return gradients; }
    vector<Point2<double>> constraintGradients() ;
};


#endif
