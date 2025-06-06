#include "Star.h"

void restrict_star_region(Star* star, double bound_x, double bound_y) 
{
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

double overlap1d(double a_start, double a_end, double b_start, double b_end) 
{
    return max(0.0, min(a_end, b_end) - max(a_start, b_start));
}