#include "Star.h"
#include "optimizer.h"



void Optimizer::updateGradients() 
{
    vector<Point2<double>> field_grads = bin_grid.getBinField();
    vector<Point2<double>> constraint_grads = constraintGradients();
    for (int i = 0; i < stars.size(); ++i) 
    {
        Star* star = stars[i];
        if (star->invalid) continue;
        //cout<<"Star: " << star->name << " Position: (" << star->position.x << ", " << star->position.y << ")" << endl;
        //cout<<"Field Gradient: (" << field_grads[i].x << ", " << field_grads[i].y << ")" << endl;
        //cout<<"Constraint Gradient: (" << constraint_grads[i].x << ", " << constraint_grads[i].y << ")" << endl;
        gradients[i] = (field_grads[i] + constraint_fac * constraint_grads[i])/sqrt(constraint_fac * constraint_fac + 1.0);
    }
    constraint_fac *= 1.1;
    constraint_fac = min(constraint_fac, 10000.0); // Limit the constraint factor to prevent overflow
}

void Optimizer::updatePositions() 
{
    for (int i = 0; i < stars.size(); ++i) 
    {
        Star* star = stars[i];
        if (star->invalid) continue;
        Point2<double> grad = gradients[i];
        double step_size = step_sizes[i];
        star->position.x -= grad.x * step_size;
        star->position.y -= grad.y * step_size;
        if(star->position.x < 0) 
        {
            star->position.x = 0; // Ensure position does not go out of bounds
        }
        if(star->position.x + star->width() > bound_x)
        {
            star->position.x = bound_x - star->width(); // Ensure position does not go out of bounds
        }
        if(star->position.y < 0)
        {
            star->position.y = 0; // Ensure position does not go out of bounds
        }
        if(star->position.y + star->height() > bound_y)
        {
            star->position.y = bound_y - star->height(); // Ensure position does not go out of bounds
        }
    }
}

bool inside_interval(double x, const pair<int, int>& interval) 
{
    return x >= interval.first && x <= interval.second;
}
bool inside_interval(const Star& star) 
{
    bool in_interval = inside_interval(star.position.x, star.observe_constraints) &&
                     inside_interval(star.position.x + star.width(), star.observe_constraints);
    bool in_moon_constraints = inside_interval(star.position.x, star.moon_constraints) &&
                              inside_interval(star.position.x + star.width(), star.moon_constraints);
    return in_interval && !in_moon_constraints;
}

Point2<double> interval_gradient(const Star& star) 
{
    if(star.invalid) return Point2<double>(0, 0);
    if(inside_interval(star)) 
    {
        return Point2<double>(0.0, 0.0); // No movement needed
    }
    if(star.position.x < star.observe_constraints.first) 
    {
        return Point2<double>(-1.0, 0); // Move right
    } 
    else if(star.position.x + star.width() > star.observe_constraints.second) 
    {
        return Point2<double>(1.0, 0); // Move left
    }

    double left_interval = star.observe_constraints.second - star.moon_constraints.second;
    double left_center = star.observe_constraints.second/2 + star.moon_constraints.second/2;
    double right_interval = star.moon_constraints.first - star.observe_constraints.first;
    double right_center = star.moon_constraints.first/2 + star.observe_constraints.first/2;

    if(abs(star.position.x - left_center) < abs(star.position.x + star.width() - right_center)) 
    {
            return Point2<double>(-1.0, 0); // Move right
    } 
    else 
    {
            return Point2<double>(1.0, 0); // Move left
    }
    
    return Point2<double>(0.0, 0); 
}

vector<Point2<double>> Optimizer::constraintGradients() 
{
    vector<Point2<double>> constraint_grads(stars.size(), Point2<double>(0, 0));
    for (int i = 0; i < stars.size(); ++i) 
    {
        Star* star = stars[i];
        if (star->invalid) continue;
        constraint_grads[i] = interval_gradient(*star);
        // Add other constraints as needed
    }
    return constraint_grads;
}

Optimizer::Optimizer(double x0, double x1, double y0, double y1, vector<Star*> stars) : bin_grid(x0, x1, y0, y1, stars) {
    cout << "Optimizer constructor called" << endl;
    this->stars = stars;
    gradients.resize(stars.size(), Point2<double>(0, 0));
    step_sizes.resize(stars.size(), 0.5); // Initial step size
    bound_x = x1 - x0;
    bound_y = y1 - y0;
}

