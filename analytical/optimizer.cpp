#include "Star.h"
#include "optimizer.h"



void Optimizer::updateGradients() 
{
    vector<Point2<double>> field_grads = bin_grid.getBinField();
    vector<Point2<double>> constraint_grads = constraintGradients();

    double max_charge_density = 0.0;
    for(int i = 0 ; i < stars.size() ; i++)
    {
        Star* star = stars[i];
        if(star->invalid) continue;
        max_charge_density = max(max_charge_density, star->charge_density());
    }

    for (int i = 0; i < stars.size(); ++i) 
    {
        Star* star = stars[i];
        if (star->invalid) continue;
        //cout<<"Star: " << star->name << " Position: (" << star->position.x << ", " << star->position.y << ")" << endl;
        //cout<<"Field Gradient: (" << field_grads[i].x << ", " << field_grads[i].y << ")" << endl;
        //cout<<"Constraint Gradient: (" << constraint_grads[i].x << ", " << constraint_grads[i].y << ")" << endl;
        gradients[i] = field_grads[i];
        if(use_constraint)
            gradients[i] = (gradients[i] + star->score/max_charge_density * constraint_fac * constraint_grads[i])/sqrt(constraint_fac * constraint_fac + 1.0);
        
    }
    /*
    cout<<gradients[rand() % gradients.size()].x << " " << gradients[rand() % gradients.size()].y << endl;
    cout<<"INTERVAL"<<endl;
    int index = rand() % stars.size();
    Star* star = stars[index];
    cout<<"Star: " << star->name << " Position: (" << star->position.x << ", " << star->position.y << ")" << endl;
    cout<<"Observe Constraints: (" << star->observe_constraints.first << ", " << star->observe_constraints.second << ")" << endl;
    cout<<"Moon Constraints: (" << star->moon_constraints.first << ", " << star->moon_constraints.second << ")" << endl;
    cout<<"Gradient: (" << gradients[index].x << ", " << gradients[index].y << ")" << endl;
    cout<<"Constraint factor: " << constraint_fac << endl;
    */
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
        step_sizes[i] *= 1.01;
        step_sizes[i] = min(step_sizes[i], 1.0); // Limit the step size to prevent overflow
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

    double moon_interval_center = (star.moon_constraints.first + star.moon_constraints.second) / 2.0;   
    double left_interval = star.observe_constraints.second - star.moon_constraints.second;
    double left_center = star.observe_constraints.second/2 + star.moon_constraints.second/2;
    double right_interval = star.moon_constraints.first - star.observe_constraints.first;
    double right_center = star.moon_constraints.first/2 + star.observe_constraints.first/2;

    if(star.position.x + star.width()/2 < moon_interval_center) 
    {
            return Point2<double>(50.0, 0); // Move left
    } 
    else 
    {
            return Point2<double>(-50.0, 0); // Move left
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
    step_sizes.resize(stars.size(), 0.001); // Initial step size
    bound_x = x1 - x0;
    bound_y = y1 - y0;
    use_constraint = false; // Default to not using constraints
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