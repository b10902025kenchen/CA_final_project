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
    normal_distribution<double> dist_y(machines / 2, double(machines) / 6.0);
    int invalid_count = 0;
    for(auto& star : stars)
    {
        invalid_count += star.invalid;
        if(star.invalid) continue; // Skip invalid stars
        star.position = Point2<double>(dist_x(rng),machines / 2); // Reset position for global placement
        restrict_star_region(&star, intervals, machines); 
    }
    optimizer->density_init(); 
    cout<<"Global Placement..."<<endl;
    // Call optimizer to perform global placement
    int plot_cnt = 0; 

    
    optimizer->setUseConstraint(true);   
    for(int i = 0 ; i < 800 ; i++)
    {
        if(i%(8) == 0)
        {
            output_graph("initial placement");
        }
        optimizer->updateGradients();
        optimizer->updatePositions();
        for(auto& star : stars)
            star.position.y = machines / 2; // Reset position for global placement
    }

    for(auto& star : stars)
    {
        if(star.invalid) continue; // Skip invalid stars
        star.position.y = dist_y(rng); // Reset position for global placement
        restrict_star_region(&star, intervals, machines); 
        break;
    }

    optimizer->setUseConstraint(true);   
    optimizer->setUseDensity(true); 
    for(int i = 0 ; i < 400 ; i++)
    {
        if(i%(8) == 0)
        {
            output_graph("electrostatic method");
            optimizer->update_constraint_fac();
        }
        optimizer->updateGradients();
        optimizer->updatePositions();
    }

    
    optimizer->set_step_size_bound(boundryRight()/20.0, boundryTop()/(2.0*machines)); // Set fixed step size for constraint optimization
    for(int i = 0 ; i < 800 ; i++)
    {
        optimizer->updateGradients();
        optimizer->updatePositions();
        optimizer->update_constraint_fac();
        if(i%(10) == 0)
        {
            output_graph("adding constraint gradient");
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

        if(star.moon_constraints.second <= star.moon_constraints.first)
        {
            star.moon_constraints.second = -1; 
            star.moon_constraints.first = -1; // Mark as invalid if moon constraints are invalid
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
    ofstream outfile(string("./plot/") + to_string(plot_cnt++), ios::out);
    
    outfile<<"set terminal pngcairo enhanced size 480,360"<<endl; 
    outfile<<"set output \'"<<to_string(plot_cnt)<<".png\'"<<endl;

    outfile << " " << endl;
    outfile << "set title \""<<outfilename<<", frame "<<plot_cnt<<"\"" << endl;
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
        plotBoxPLT(outfile, module.x(), module.y()*10+1, module.x() + module.width() - switch_time, (module.y() + module.height())*10-1);
    }
    outfile << "EOF" << endl;
    outfile << "unset output" << endl;
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

std::vector<int> Star_Placement::WIS_single_row(std::vector<Star*>& t, double& best)
{
    if (t.empty()) { best = 0.0; return {}; }


    std::sort(t.begin(), t.end(),
              [&](const Star* a, const Star* b){ 
                if (int(a->x()) < int(b->x())) return true;
                if (int(a->x()) > int(b->x())) return false;
                return a->score / a->area() > b->score / b->area();
            });
    list<Star*> chosen_tasks;
    vector<int> chose_tasks_idx(t.size(), 0);

    best = 0.0;
    chosen_tasks.push_back(t[0]); 
    int current_endtime = t[0]->x() + t[0]->w; 
    for(int i = 1; i < t.size(); ++i) {
        if(t[i]->x() >= current_endtime) {
            chosen_tasks.push_back(t[i]);
            int new_loc = max(current_endtime, t[i]->observe_constraints.first);
            if(t[i]->x() > t[i]->moon_constraints.second) 
                new_loc = max(new_loc, t[i]->moon_constraints.second);
            if(new_loc + t[i]->width() > t[i]->observe_constraints.second) continue; // Skip if star exceeds observe constraints
            if(new_loc + t[i]->width() > t[i]->moon_constraints.first && new_loc < t[i]->moon_constraints.first) continue; 
            t[i]->position.x = new_loc;
            current_endtime = t[i]->x() + t[i]->w; // Update end time
            best += t[i]->score; // Update score
            chose_tasks_idx[i] = 1; // Mark this task as chosen
        } 
    }
    
    vector<Star*> left_stars;
    left_stars.reserve(t.size());
    for(int i = 0; i < t.size(); ++i) {
        if(chose_tasks_idx[i] == 0) {
            left_stars.push_back(t[i]); // Collect unchosen stars
        }
    }

    list<pair<int, int>> intervals;
    for (auto it = chosen_tasks.begin(); it != chosen_tasks.end(); ++it) {
        auto curr = it;
        if ((*curr)->invalid) continue;    

        auto next = std::next(curr);
        if (next == chosen_tasks.end()) break;   
        if ((*next)->invalid) continue;     

        int start = (*curr)->position.x + (*curr)->width();
        int end   = (*next)->position.x;
        intervals.emplace_back(start, end);
    }

    sort(left_stars.begin(), left_stars.end(), [](const Star* a, const Star* b) {
        return a->score / a->area() > b->score / b->area();
    });
    

    
    
    vector<int> chosen_indices;
    for(auto& star : chosen_tasks) {
        if(star->invalid) continue; // Skip invalid stars
        chosen_indices.push_back(star - &stars[0]); // Get index of the star in the original vector
    }
    return chosen_indices;
}

void Star_Placement::legalization() {
    round();
    set_load();
    vector<vector<Star*>> tasks;
    tasks.resize(machines);
    
    for(int i = 0; i < machines; ++i) {
        tasks[i].reserve(machines_load[i].size());
        for(auto& star : machines_load[i]) {
            star->invalid = !inside_interval(*star); 
            if(star->invalid) continue; // Skip invalid stars
            tasks[i].push_back(star);
        }
        output_graph("rounding");
    }

    for(int i = 0 ; i < stars.size() ; i++)
    {
        if(stars[i].invalid) continue; // Skip invalid stars
        cout<<"Star "<<stars[i].name<<" with observation time "<<stars[i].w<<" and score "<<stars[i].score<<endl;
        cout<<"Position: ("<<stars[i].x()<<", "<<stars[i].y()<<")"<<endl;
        cout<<"Observe Constraints: ("<<stars[i].observe_constraints.first<<", "<<stars[i].observe_constraints.second<<")"<<endl;
        cout<<"Moon Constraints: ("<<stars[i].moon_constraints.first<<", "<<stars[i].moon_constraints.second<<")"<<endl;
        if(i% (stars.size()/10) == 0)
            output_graph("remove illegal schedule");
    }
    
    
    vector<vector<int>> chosens;
    chosens.resize(machines);
    double overall_score = 0.0;
    for(int i = 0; i < machines; ++i) {
        double score = 0.0;
        chosens[i] = WIS_single_row(tasks[i],score);
        for(auto& star : machines_load[i]) {
            star->invalid = true; // Mark all stars as invalid first
        }
        for(int idx : chosens[i]) {
            if(idx >= 0 && idx < stars.size()) {
                stars[idx].invalid = false; // Mark chosen stars as valid
            }
        }
        output_graph("legalization");
        cout << "Machine " << i << " score: " << score << endl;
        overall_score += score;
    }

    cout<< "Overall score after WIS: " << overall_score << endl;

    

}