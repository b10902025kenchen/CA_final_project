#include "placement_base.h"

string extension(const string& filename) {
    size_t dot_pos = filename.find_last_of('.');
    if (dot_pos == string::npos) return "";
    return filename.substr(dot_pos + 1);
}

void Star_Placement::global_placement()
{
    for(auto& star : stars)
    {
        if(star.invalid) continue; // Skip invalid stars
        star.position = Point2<double>(intervals/2 + rand()%101 - 50,machines/2 + rand()%3-1); // Reset position for global placement
    }
    cout<<"Global Placement..."<<endl;
    // Call optimizer to perform global placement
    for(int i = 0 ; i < 2000 ; i++)
    {
        optimizer->updateGradients();
        optimizer->updatePositions();
        if(i%200 == 0)
        {
            string graph_filename = "./plot/placement_graph_" + to_string(i) + ".txt";
            output_graph(graph_filename);
        }
    }
    
    for(int i = 0 ; i < stars.size() ; i++)
    {
        if(stars[i].invalid) continue; // Skip invalid stars
        cout<<"Star "<<stars[i].name<<" placed at ("<<stars[i].x()<<", "<<stars[i].y()<<")"<<endl;
    }
    cout<<"Global Placement Complete."<<endl;
}

Star_Placement::Star_Placement(int argc, char* argv[])
{
    cout<<"Star Placement Initialization..."<<endl;
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

    cout<<"Reading stars from "<<star_info<<endl;
    ifstream star_file(star_info);
    list<Star> star_list;
    string line;
    // format -> name, observe_constraints.first, observe_constraints.second, moon_constraints.first, moon_constraints.second
    
    while(getline(star_file, line)) {
        if(line.empty()) continue;
        Star star;
        istringstream iss(line);
        //iss >> star.name >> star.observe_constraints.first >> star.observe_constraints.second
        //    >> star.moon_constraints.first >> star.moon_constraints.second;   wrong
        getline(iss, star.name, ',');
        string obs_start, obs_end, moon_start, moon_end;
        getline(iss, obs_start, ',');
        getline(iss, obs_end, ',');
        getline(iss, moon_start, ',');
        getline(iss, moon_end, ',');
        star.observe_constraints.first = stoi(obs_start);
        star.observe_constraints.second = stoi(obs_end);
        star.moon_constraints.first = stoi(moon_start);
        star.moon_constraints.second = stoi(moon_end);
        star.position = Point2<double>(0, 0); // Initial position
        star_list.push_back(star);
    }
    stars.assign(star_list.begin(), star_list.end());
    star_file.close();

    ifstream param_file(parameter);
    /*
    format is as follows:

    MACHINES 2
    INTERVALS 400
    SWITCH_TIME 5
    
    */
    cout<<"Reading parameters from "<<parameter<<endl;
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

    cout<<"Read observation from "<<observation_file<<endl;
    ifstream obs_file(observation_file);
    double obs_value;
    int i = 0;
    while(obs_file >> obs_value) {
        stars[i++].w = obs_value + switch_time; // Set observation time for each star
    }

    obs_file.close();

    cout<<"Read score from "<<score_file<<endl;
    ifstream score_file_stream(score_file);
    double score_value;
    i = 0;
    while(score_file_stream >> score_value) {
        stars[i++].score = score_value; // Set score for each star
    }


    for(auto& star : stars)
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
    for(auto& star : stars) {
        if(star.invalid)  // Skip invalid stars
        cout<<"Star "<<star.name<<" with observation time "<<star.w<<" and score "<<star.score<<endl;
    }

    vector<Star*> stars_ptr;
    stars_ptr.reserve(stars.size());
    for(auto& star : stars) {
        stars_ptr.push_back(&star);
    }
    cout<<"intervals: "<<intervals<<", machines: "<<machines<<", stars: "<<stars.size()<<endl;
    optimizer = new Optimizer(0, intervals, 0, machines, stars_ptr);

    cout<<"Star Placement Initialization Complete."<<endl;
}