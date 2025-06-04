#include "placement_base.h"
#include <random>
string extension(const string& filename) {
    size_t dot_pos = filename.find_last_of('.');
    if (dot_pos == string::npos) return "";
    return filename.substr(dot_pos + 1);
}

void Star_Placement::global_placement()
{
    //normal distribution for initial positions
    mt19937 rng(1);
    normal_distribution<double> dist_x(intervals / 2, intervals / 20);
    normal_distribution<double> dist_y(machines / 2, 1.0);
    int invalid_count = 0;
    for(auto& star : stars)
    {
        invalid_count += star.invalid;
        if(star.invalid) continue; // Skip invalid stars
        star.position = Point2<double>(dist_x(rng),dist_y(rng)); // Reset position for global placement
        if(star.position.x < 0) star.position.x = 0; // Ensure position does not go out of bounds
        if(star.position.x + star.width() > intervals)
            star.position.x = intervals - star.width(); // Ensure position does not go out of bounds
        if(star.position.y < 0) star.position.y = 0; // Ensure position does not go out of bounds
        if(star.position.y + star.height() > machines)
            star.position.y = machines - star.height(); // Ensure position does not go out of bounds
        
    }
    cout<<"Global Placement..."<<endl;
    // Call optimizer to perform global placement
    
    for(int i = 0 ; i < stars.size() ; i++)
    {
        if(i%(4) == 0)
        {
            string graph_filename = "./plot/placement_graph_" + to_string(i) + ".txt";
            output_graph(graph_filename);
        }
        optimizer->updateGradients();
        optimizer->updatePositions();
        
        
    }

    optimizer->setUseConstraint(true); 
    for(int i = 0 ; i < 1000 ; i++)
    {
        optimizer->updateGradients();
        optimizer->updatePositions();
        optimizer->update_constraint_fac();
        if(i%(stars.size()/5) == 0)
        {
            string graph_filename = "./plot/placement_graph_" + to_string(i+stars.size()) + ".txt";
            output_graph(graph_filename);
        }
    }
    /*
    for(int i = 0 ; i < stars.size() ; i++)
    {
        if(stars[i].invalid) continue; // Skip invalid stars
        cout<<"Star "<<stars[i].name<<" placed at ("<<stars[i].x()<<", "<<stars[i].y()<<")"<<endl;
    }
    */
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
    getline(star_file, line);
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

    if(observation_file != "")
    {
        cout<<"Read observation from "<<observation_file<<endl;
        ifstream obs_file(observation_file);
        double obs_value;
        int i = 0;
        while(obs_file >> obs_value) {
            stars[i++].w = obs_value + switch_time; // Set observation time for each star
        }
        obs_file.close();
    }
    else
    {
        cout<<"No observation file provided, using default observation time of 10 for each star."<<endl;
        for(auto& star : stars) {
            star.w = 10 + switch_time; // Default observation time
        }
    }

    

    if(score_file != "")
    {
        cout<<"Read score from "<<score_file<<endl;
        ifstream score_file_stream(score_file);
        double score_value;
        int i = 0;
        while(score_file_stream >> score_value) {
            stars[i++].score = score_value; // Set score for each star
        }
        score_file_stream.close();
    }
    else
    {
        cout<<"No score file provided, using default score of 1 for each star."<<endl;
        for(auto& star : stars) {
            star.score = 1.0; // Default score
        }
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
    //constraint dbg
    /*
    for(auto& star : stars) {
        if(star.invalid)  // Skip invalid stars
        cout<<"Star "<<star.name<<" with observation time "<<star.w<<" and score "<<star.score<<endl;
    }
        */
    machines_load.resize(machines);
    vector<Star*> stars_ptr;
    stars_ptr.reserve(stars.size());
    for(auto& star : stars) {
        stars_ptr.push_back(&star);
    }
    cout<<"intervals: "<<intervals<<", machines: "<<machines<<", stars: "<<stars.size()<<endl;
    optimizer = new Optimizer(0, intervals, 0, machines, stars_ptr);

    cout<<"Star Placement Initialization Complete."<<endl;
}

void plotBoxPLT(ofstream &stream, double x1, double y1, double x2, double y2) {
    stream << round(x1) << ", " << round(y1) << endl
           << round(x2) << ", " << round(y1) << endl
           << round(x2) << ", " << round(y2) << endl
           << round(x1) << ", " << round(y2) << endl
           << round(x1) << ", " << round(y1) << endl
           << endl;
}

void Star_Placement::output_graph(const string& outfilename) {
    ofstream outfile(outfilename.c_str(), ios::out);
    outfile << " " << endl;
    //outfile << "set title \"wirelength = " << _placement.computeHpwl() << "\"" << endl;
    outfile << "set size ratio 0.78" << endl;
    outfile << "set nokey" << endl
            << endl;
    outfile << "plot[:][:] '-' w l lt 3 lw 2, '-' w l lt 1" << endl
            << endl;
    outfile << "# bounding box" << endl;
    plotBoxPLT(outfile, boundryLeft(), boundryBottom()*10, boundryRight(), boundryTop()*10);
    outfile << "EOF" << endl;
    outfile << "# modules" << endl
            << "0.00, 0.00" << endl
            << endl;
    for (size_t i = 0; i < numStars(); ++i) {
        Star &module = stars[i];
        if(module.invalid) continue; // Skip invalid stars
        plotBoxPLT(outfile, module.x(), module.y()*10+1, module.x() + module.width(), (module.y() + module.height())*10-1);
    }
    outfile << "EOF" << endl;
    outfile << "pause -1 'Press any key to close.'" << endl;
    outfile.close();

}

void Star_Placement::round() {
    for(auto& star : stars) {
        if(star.invalid) continue; // Skip invalid stars
        star.position.x = std::round(star.position.x);
        star.position.y = std::round(star.position.y);
    }
}

void Star_Placement::set_load()
{
    for(int i = 0; i < machines_load.size(); ++i) {
        machines_load[i].clear();
    }
    for(auto& star : stars) {
        if(star.invalid) continue; // Skip invalid stars
        int machine_idx = int(star.y()); // Round to nearest machine index
        machines_load[machine_idx].push_back(&star);
    }
    for(int i = 0; i < machines_load.size(); ++i) {
        vector<Star*> machine_stars(machines_load[i].begin(), machines_load[i].end());

        sort(machine_stars.begin(), machine_stars.end(), [](Star* a, Star* b) {
            return a->x() < b->x(); // Sort by x-coordinate
        });

        machines_load[i].clear();
        machines_load[i].assign(machine_stars.begin(), machine_stars.end());
        cout<<"Machine "<<i<<" has "<<machines_load[i].size()<<" stars."<<endl;
    }
}

double ovlp(const Star& a, const Star& b) {
    double x_overlap = std::max(0.0, std::min(a.x() + a.width(), b.x() + b.width()) - std::max(a.x(), b.x()));
    double y_overlap = std::max(0.0, std::min(a.y() + a.height(), b.y() + b.height()) - std::max(a.y(), b.y()));
    return x_overlap * y_overlap;
}

std::vector<int> Star_Placement::WIS_single_row(std::vector<Task>& t, double& best)
{
    if (t.empty()) { best = 0.0; return {}; }


    std::sort(t.begin(), t.end(),
              [&](const Task& a, const Task& b){ return stars[a.idx].charge_density() > stars[b.idx].charge_density(); });

    list<int> chosen_tasks;
    chosen_tasks.push_back(t[0].idx); 
    for(int i = 1; i < t.size(); ++i) {
        bool conflict = false;
        for(int j : chosen_tasks) {
            if(ovlp(stars[t[i].idx], stars[j]) > 0) {
                conflict = true;
                break;
            }
        }
        if(!conflict) {
            chosen_tasks.push_back(t[i].idx);
        }
    }
    best = 0.0;
    for(int idx : chosen_tasks) {
        best += stars[idx].score; // Sum up the scores of the chosen stars
    }

    return vector<int>(chosen_tasks.begin(), chosen_tasks.end());
}

void Star_Placement::legalization() {
    round();
    set_load();
    vector<vector<Task>> tasks;
    tasks.resize(machines);
    output_graph("./plot/rounding.txt");
    for(int i = 0; i < machines; ++i) {
        tasks[i].reserve(machines_load[i].size());
        for(auto& star : machines_load[i]) {
            star->invalid = !inside_interval(*star); 
            if(star->invalid) continue; // Skip invalid stars
            Task task;
            task.start = star->observe_constraints.first;
            task.end = star->observe_constraints.second;
            task.score = star->score;
            task.idx = star - &stars[0]; // Get index of the star in the stars vector
            tasks[i].push_back(task);
        }
    }

    for(int i = 0 ; i < stars.size() ; i++)
    {
        if(stars[i].invalid) continue; // Skip invalid stars
        cout<<"Star "<<stars[i].name<<" with observation time "<<stars[i].w<<" and score "<<stars[i].score<<endl;
        cout<<"Position: ("<<stars[i].x()<<", "<<stars[i].y()<<")"<<endl;
        cout<<"Observe Constraints: ("<<stars[i].observe_constraints.first<<", "<<stars[i].observe_constraints.second<<")"<<endl;
        cout<<"Moon Constraints: ("<<stars[i].moon_constraints.first<<", "<<stars[i].moon_constraints.second<<")"<<endl;
    }
    output_graph("./plot/remove_outofbound.txt");
    vector<vector<int>> chosens;
    chosens.resize(machines);
    for(int i = 0; i < machines; ++i) {
        double score = 0.0;
        chosens[i] = WIS_single_row(tasks[i],score);
        cout << "Machine " << i << " score: " << score << endl;
    }

    for(int i = 0 ; i < machines; ++i) {
        for(auto& star : machines_load[i]) {
            star->invalid = true; // Mark all stars as invalid first
        }
        for(int idx : chosens[i]) {
            if(idx >= 0 && idx < stars.size()) {
                stars[idx].invalid = false; // Mark chosen stars as valid
            }
        }
    }
    output_graph("./plot/legalization.txt");

}