#ifndef _PLACEMENT_BASE_H
#define _PLACEMENT_BASE_H   

#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include "Point.h"
#include "BinGrid.h"
#include "optimizer.h"
using namespace std;


class Star_Placement
{
    public:
        Star_Placement(int argc, char* argv[]);
        ~Star_Placement() {cout<<endl;}

        void global_placement();
        void legalization();

        int numStars() const { return stars.size(); }
        Star& star(int index)  { return stars[index]; }
        double boundryLeft() const { return 0; } 
        double boundryRight() const { return intervals; } 
        double boundryTop() const { return machines; } 
        double boundryBottom() const { return 0; } 
        void output_graph(const string& filename) const {
            ofstream file(filename);
            double factor = double(intervals) / double(machines) / 1.6; // Scale factor for Y-axis
            for (const auto& star : stars) {
                if(star.invalid) continue; // Skip invalid stars
                file << star.name << " " <<  star.x() << " " << factor*star.y() <<" "<<( star.x() + star.width()) << " "<<factor<< endl;
            }
            file.close();
        }
    private:
        vector<Star>   stars; // list of stars
        int          machines; // number of machines    (Y-axis)
        int          switch_time; // switch time between stars
        int          intervals; // number of intervals (X-axis)
        Optimizer* optimizer; // optimizer for placement
        
    
};



#endif