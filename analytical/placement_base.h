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

struct Task {
    int  start, end;  
    double score;
    int  idx;          
};

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
        void output_graph(const string& filename);
    private:
        void set_load();
        void round();
        std::vector<int> WIS_single_row(vector<Star*>& t,double& best);
        vector<Star>   stars; // list of stars
        vector<list<Star*>> machines_load;
        int          machines; // number of machines    (Y-axis)
        int          switch_time; // switch time between stars
        int          intervals; // number of intervals (X-axis)
        Optimizer* optimizer; // optimizer for placement

        int plot_cnt = 0; // plot count for graph output
        
    
};



#endif