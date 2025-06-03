#include "Star.h"
#include "optimizer.h"

void Optimizer::updateGradients() 
{
    vector<Point2<double>> field_grads = bin_grid.getBinField_cache();
    vector<Point2<double>> constraint_grads(placement.numStars(), Point2<double>(0, 0));
}

Point2<double> interval_gradient(const Star& star) //bell shaped method
{
    
    return Point2<double>(1.0, 1.0); 
}

vector<Point2<double>> Optimizer::constraintGradients() 
{
    vector<Point2<double>> constraint_grads(placement.numStars(), Point2<double>(0, 0));
    for (int i = 0; i < placement.numStars(); ++i) 
    {
        const Star& star = placement.star(i);
        if (star.invalid) continue;
        constraint_grads[i] = interval_gradient(star);
        // Add other constraints as needed
    }
    return constraint_grads;
}

Optimizer::Optimizer(Star_Placement& placement): placement(placement), bin_grid(placement) 
{
    stars.resize(placement.numStars());
    for (int i = 0; i < placement.numStars(); ++i) 
        stars[i] = &placement.star(i);
    
    gradients.resize(placement.numStars(), Point2<double>(0, 0));
    step_sizes.resize(placement.numStars(), 1.0); 
}

