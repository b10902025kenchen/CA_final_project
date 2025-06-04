#include "placement_base.h"

int main(int argc, char* argv[]) 
{
    Star_Placement placement(argc, argv);
    placement.global_placement();
    placement.legalization();
    return 0;
}