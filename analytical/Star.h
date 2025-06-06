#ifndef _STAR_H
#define _STAR_H   

#include <iostream>
#include <fstream>
#include <vector>
#include "Point.h"

using namespace std;

struct Star
{
    string name;
    int w;  // observation time length 
    pair<int,int> observe_constraints; // [start, end] time constraints
    pair<int,int> moon_constraints; // [start, end] time constraints
    double score = 1; // score of the star
    bool invalid = false; // if the star is invalid
    Point2<double> position; // (Machine, interval) space
    double x() const { return position.x; }
    double y() const { return position.y; }
    double width() const { return w; }
    double charge_density() const { return score/area(); } // Charge density is the score of the star
    double height() const { return 1; } // Machine is 1 unit high
    double area() const { return width() * height(); }
};

void restrict_star_region(Star* star, double bound_x, double bound_y);
double overlap1d(double a_start, double a_end, double b_start, double b_end) ;
Point2<double> interval_gradient(const Star& star);




#endif