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
        gradients[i] = density_fac * field_grads[i];
        if(use_constraint)
            gradients[i] = (gradients[i] + star->score/max_charge_density * constraint_fac * constraint_grads[i])/sqrt(constraint_fac * constraint_fac + 1.0);
        
    }
    
    constraint_fac = min(constraint_fac, 10000.0); // Limit the constraint factor to prevent overflow
}



void Optimizer::updatePositions() 
{
    for(int i = 0; i < stars.size(); ++i) 
    {
        Star* star = stars[i];
        prev_gradients[i] = gradients[i];
        prev_positions[i] = star->position;
    }

    step_size_prediction();
    
    for(int i = 0 ; i < step_sizes_bound.size(); ++i) {
        if(gradients[i].x * step_sizes[i].x > step_sizes_bound[i].x) 
            step_sizes[i].x = step_sizes_bound[i].x / gradients[i].x;
        if(gradients[i].y * step_sizes[i].y > step_sizes_bound[i].y)
            step_sizes[i].y = step_sizes_bound[i].y / gradients[i].y;
    }

    for (int i = 0; i < stars.size(); ++i) 
    {
        Star* star = stars[i];
        if (star->invalid) continue;
        Point2<double> grad = gradients[i];
        Point2<double> step_size = step_sizes[i];
        star->position.x -= grad.x * step_size.x;
        star->position.y -= grad.y * step_size.y;
        restrict_star_region(star, bound_x, bound_y);
    }

}

void Optimizer::step_size_prediction()
{
    for(int i = 0 ; i < stars.size(); ++i) 
    {
        if(stars[i]->invalid) continue;

        step_sizes[i].x = abs(prev_positions[i].x - stars[i]->position.x) / abs(prev_gradients[i].x - gradients[i].x);
        step_sizes[i].y = abs(prev_positions[i].y - stars[i]->position.y) / abs(prev_gradients[i].y - gradients[i].y);
        if(abs(prev_gradients[i].x - gradients[i].x) == 0)
            step_sizes[i].x = 0.5;
        if(abs(prev_gradients[i].y - gradients[i].y) == 0)
            step_sizes[i].y = 0.5;
        step_sizes[i] *= 0.5;
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
    bool in_moon_constraints = inside_interval(star.position.x, star.moon_constraints) ||
                              inside_interval(star.position.x + star.width(), star.moon_constraints);
    return in_interval && !in_moon_constraints;
}

double soft_plus(double x,double a) 
{
    return 1/a*log(1 + exp(a*x));
}

double sigmoid(double x) 
{
    return 1 / (1 + exp(-x));
}

double soft_plus_gradient(double x) 
{
    return 1 / (1 + exp(-x));
}

double overlap1d(double a_start, double a_end, double b_start, double b_end) 
{
    return max(0.0, min(a_end, b_end) - max(a_start, b_start));
}

Point2<double> interval_gradient(const Star& star) 
{
    double machine_gradient = 0.005*(star.position.y - round(star.position.y)); 
    double constraint_gradient = 0;
    if(star.invalid) return Point2<double>(0, 0);

    if(star.position.x < star.observe_constraints.first) 
    {
        constraint_gradient -=1;
    } 
    else if(star.position.x + star.width() > star.observe_constraints.second) 
    {
        constraint_gradient +=1;
    }

    double moon_interval_center = (star.moon_constraints.first + star.moon_constraints.second) / 2.0;   
    double left_interval = star.observe_constraints.second - star.moon_constraints.second;
    double left_center = star.observe_constraints.second/2 + star.moon_constraints.second/2;
    double right_interval = star.moon_constraints.first - star.observe_constraints.first;
    double right_center = star.moon_constraints.first/2 + star.observe_constraints.first/2;

    if(star.position.x + star.width()/2 < moon_interval_center) 
    {
        constraint_gradient+=1;
    } 
    else 
    {
        constraint_gradient-=1;
    }
    
    return Point2<double>(constraint_gradient, machine_gradient); 
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
    prev_gradients.resize(stars.size(), Point2<double>(0, 0));
    prev_positions.resize(stars.size(), Point2<double>(0, 0));
    step_sizes.resize(stars.size(), Point2<double>(0.001,0.001)); // Initial step size
    step_sizes_bound.resize(stars.size(), Point2<double>((x1-x0)/5.0, (y1-y0)/5.0)); 
    bound_x = x1 - x0;
    bound_y = y1 - y0;
    use_constraint = false; // Default to not using constraints

    max_charge_density = 0.0;
    for(int i = 0 ; i < stars.size() ; i++)
    {
        Star* star = stars[i];
        if(star->invalid) continue;
        max_charge_density = max(max_charge_density, star->charge_density());
    }
}

void Optimizer::normalize_step_sizes()
{
    double allowed_max_step_size_x = bound_x / 10.0; // Limit step size to 10% of the total width
    double allowed_max_step_size_y = bound_y / 10.0; // Limit step size to 10% of the total height

    double max_x_grad = 0.0;
    double max_y_grad = 0.0;
    for (const auto& grad : gradients) {
        max_x_grad = max(max_x_grad, abs(grad.x));
        max_y_grad = max(max_y_grad, abs(grad.y));
    }

    for(int i = 0; i < step_sizes.size(); ++i) {
        step_sizes[i] = 0;
    }
    for (int i = 0; i < step_sizes.size(); ++i) {
        Star* star = stars[i];
        if (star->invalid) continue;
        double grad_x = gradients[i].x;
        double grad_y = gradients[i].y;

        if (max_x_grad == 0.0 || max_y_grad == 0.0) {
            step_sizes[i] = 0.1; // Default step size if gradients are zero
            continue;
        }
        step_sizes[i] = min( allowed_max_step_size_x / max_x_grad, allowed_max_step_size_y / max_y_grad);
    }
}