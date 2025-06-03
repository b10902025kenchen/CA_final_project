#include "placement_base.h"

string extension(const string& filename) {
    size_t dot_pos = filename.find_last_of('.');
    if (dot_pos == string::npos) return "";
    return filename.substr(dot_pos + 1);
}

Star_Placement::Star_Placement(int argc, char* argv[]):optimizer(*this)
{
    string star_info;
    string parameter;
    string observation_file;
    string score_file;
    for(int i = 1 ; i < argc ; i++)
    {
        string arg = argv[i];
        if(extension(arg) == "csv")
            star_info = arg;
        else if(arg == "--parameter" || arg == "-p")
            parameter = argv[++i];
        else if(arg == "--observation" || arg == "-o")
            observation_file = argv[++i];
        else if(arg == "--score" || arg == "-s")
            score_file = argv[++i];
    }

    ifstream star_file(star_info);
    list<Star> star_list;
    string line;
    while(getline(star_file, line)) {
        if(line.empty()) continue;
        Star star;
        istringstream iss(line);
        iss >> star.name >> star.w >> star.observe_constraints.first >> star.observe_constraints.second
            >> star.moon_constraints.first >> star.moon_constraints.second;
        star.position = Point2<int>(0, 0); // Initial position
        star_list.push_back(star);
    }
    stars.assign(star_list.begin(), star_list.end());
    star_file.close();

    ifstream param_file(parameter);
    /*
    format is as follows:

    MACHINES 2
    INTERVALS 10
    SWITCH_TIME 5
    
    */

    string param_line;
    while(getline(param_file, param_line)) {
        if(param_line.empty()) continue;
        istringstream iss(param_line);
        string key;
        int value;
        iss >> key >> value;
        if(key == "MACHINES") {
            machines = value;
        } else if(key == "INTERVALS") {
            intervals = value;
        } else if(key == "SWITCH_TIME") {
            switch_time = value;
        }
    }

    param_file.close();

    ifstream obs_file(observation_file);
    double obs_value;
    int i = 0;
    while(obs_file >> obs_value) {
        stars[i++].w = obs_value + switch_time; // Set observation time for each star
    }

    obs_file.close();

    ifstream score_file_stream(score_file);
    double score_value;
    i = 0;
    while(score_file_stream >> score_value) {
        stars[i++].score = score_value; // Set score for each star
    }


    for(auto star : stars)
    {
        int left_interval = star.observe_constraints.second - star.moon_constraints.second;
        int right_interval = star.moon_constraints.first - star.observe_constraints.first;

        if(left_interval < star.w)
        {
            star.moon_constraints.second = star.observe_constraints.second;
        }
        if(right_interval < star.w)
        {
            star.moon_constraints.first = star.observe_constraints.first;
        }
        if(star.moon_constraints.first == star.observe_constraints.first\
            && star.moon_constraints.second == star.observe_constraints.second)
        {
            star.invalid = true; // Mark star as invalid if no valid observation time
        }
    }
}