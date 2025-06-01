#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <limits>
#include <algorithm>
#include <random>
#include <omp.h>
#include "weight_int_sch.h"

using namespace std;

int threads = 1;
int MACHINES = 1;
bool VAR_SCORE = false;
bool VAR_TIME = false;
const int STARS = 110;
const int RANGE = 448;
const int OBSERVATION_TIME = 10;
const int SWITCH_TIME = 5;
const int TOTAL_TIME = OBSERVATION_TIME + SWITCH_TIME;
const int INTERVALS = RANGE / TOTAL_TIME;
vector<double> observation = {
    30, 30, 30, 20, 20, 20, 20, 30, 30, 30, 30, 30, 20, 30, 30, 30, 30, 30, 30, 40,
    30, 20, 30, 20, 30, 30, 30, 30, 30, 30, 20, 30, 20, 20, 20, 30, 30, 30, 20, 40,
    20, 20, 40, 20, 20, 30, 20, 20, 30, 30, 30, 30, 30, 30, 30, 30, 30, 40, 40, 30,
    40, 30, 30, 30, 40, 30, 30, 30, 30, 30, 30, 40, 40, 40, 30, 60, 30, 30, 30, 30,
    30, 30, 30, 40, 40, 30, 30, 40, 40, 40, 60, 30, 30, 30, 40, 40, 40, 60, 40, 40,
    30, 40, 30, 30, 40, 30, 30, 60, 40, 30
};

vector<vector<int>> read_csv(const string& filename) {
    vector<vector<int>> data;
    ifstream file(filename);
    string line;
    bool first_line = true;

    while (getline(file, line)) {
        if (first_line) {
            first_line = false;
            continue;
        }

        vector<int> row;
        stringstream ss(line);
        string cell;
        int col_index = 0;

        while (getline(ss, cell, ',')) {
            if (col_index > 0) { // skip first column
                row.push_back(stoi(cell));
            }
            ++col_index;
        }

        data.push_back(row);
    }

    return data;
}

bool valid(const vector<vector<int>>& data, int i, int j) {
    int start = j * TOTAL_TIME;
    int end = (j + 1) * TOTAL_TIME;

    if (data[i][0] != -1 && start >= data[i][0] && end <= data[i][1]) {
        if (data[i][2] == -1 || 
            (start <= data[i][2] && end <= data[i][2]) || 
            (start >= data[i][3] && end >= data[i][3])) {
            return true;
        }
    }

    return false;
}

template<typename T>
vector<T> star_rank_setup() {
    vector<T> scores(STARS, 1);
    if(!VAR_SCORE) {
        for (int i = 0; i < STARS; ++i) {
            scores[i] = 1; // Default score
        }
        return scores;
    }

    mt19937 rng(1);
    uniform_int_distribution<int> dist(1, 5);
    for (int i = 0; i < STARS; ++i) {
        scores[i] = dist(rng);
    }

    cout<<"Scores:"<<endl;
    for(int i = 0; i < STARS; ++i) {
        cout<<scores[i]<<" ";
    }
    cout<<endl;

    return scores;
}

template<typename T>    
vector<int> sort_by_score(vector<T>& scores) {
    vector<int> ordered(STARS);
    for (int i = 0; i < STARS; ++i) 
        ordered[i] = i;

    if(!VAR_SCORE) return ordered;

    sort(ordered.begin(), ordered.end(), [&](int a, int b) {
        return scores[a] > scores[b];
    });

    cout << "Sorted star indices:\n";
    for (int i : ordered) {
        cout << i << " ";
    }
    cout << endl;

    return ordered;
}

void start_end_setup(const vector<vector<int>>& data, vector<int>& total, vector<int>& start_time, vector<int>& end_time, int& invalid) 
{

    for (int i = 0; i < STARS; ++i) {
        total[i] = observation[i] + SWITCH_TIME;
    }

    int num = 0;
    invalid = 0;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, RANGE);

    for (int i = 0; i < STARS; ++i) {
        if (data[i][0] == -1 || (data[i][3] - data[i][2]) >= (RANGE - total[i])) {
            start_time[i] = -1;
            end_time[i] = -1;
            invalid++;
            continue;
        }
        if (num + total[i] >= RANGE) {
            num = 0;
        }
        if (valid(data, i, num, num + total[i])) {
            start_time[i] = num;
            end_time[i] = num + total[i];
            num = end_time[i];
        } else {
            start_time[i] = -1;
            end_time[i] = -1;
            bool found = false;
            for (int t = 0; t < 100; ++t) {
                int tmp = dis(gen) % (RANGE - (int)total[i]);
                if (valid(data, i, tmp, tmp + (int)total[i])) {
                    start_time[i] = tmp;
                    end_time[i] = tmp + (int)total[i];
                    found = true;
                    break;
                }
            }
            if (!found) {
                start_time[i] = -1;
                end_time[i] = -1;
                invalid++;
            }
        }
    }

    for (int i = 0; i < STARS; ++i) {
        cout << start_time[i] << " " << end_time[i] << " " << total[i] << endl;
    }
}

int main(int argc, char* argv[]) {
    
    for(int i=1;i<argc;i++)
    {
        string arg = argv[i];
        if((arg=="--telescopes"||arg=="-m") && i+1<argc)
            MACHINES = stoi(argv[++i]); 
        else if(arg=="var-score" || arg=="-s")
            VAR_SCORE = true;
        else if(arg=="-var-time" || arg == "-t")
            VAR_TIME = true;
        else if(arg=="-thread" || arg=="-mult")
            threads = min(stoi(argv[++i]),omp_get_max_threads());
    }
    omp_set_num_threads(threads);

    string filename = "./data/data.csv";
    vector<vector<int>> data = read_csv(filename);

    vector<double> scores = star_rank_setup<double>();
    vector<int> ordered_star = sort_by_score<double>(scores);
    
    int invalid = 0;
    vector<int> total(STARS);
    vector<int> start_time(STARS, 0);
    vector<int> end_time(STARS, 0);
    start_end_setup(data, total, start_time, end_time, invalid);
    if(VAR_TIME)
    {
        cout << "Number of invalid intervals: " << invalid << endl;
        double max_weight = weighted_interval_scheduling(start_time, end_time, scores, MACHINES);
        cout << "Maximum weighted interval scheduling total score: " << max_weight << endl;
        return 0;
    }


    vector<vector<int>> array(STARS, vector<int>(INTERVALS, 0));
    for (int i = 0; i < STARS; ++i) {
        for (int j = 0; j < INTERVALS; ++j) {
            if (valid(data, i, j)) {
                array[i][j] = 1;
            }
        }
    }

    // Compute column sums
    vector<int> column_sum(INTERVALS, 0);
    for (int j = 0; j < INTERVALS; ++j) {
        for (int i = 0; i < STARS; ++i) {
            column_sum[j] += array[i][j];
        }
    }

    cout << "Column Sums:\n";
    for (int j = 0; j < INTERVALS; ++j) {
        cout << column_sum[j] << " ";
    }
    cout << "\n";

    // Scheduling
    vector<vector<int>> schedule(INTERVALS);  // schedule[interval] = list of stars
    vector<int> capacity(INTERVALS, 0);
    int pts = 0;

    for (int i = 0; i < STARS; ++i) {
        int star_index = ordered_star[i];

        int smallest_sum = numeric_limits<int>::max();
        int selected_interval = -1;

        #pragma omp parallel                                   \
        default(none)                                      \
        shared(array, column_sum, capacity, smallest_sum, selected_interval, MACHINES) \
        firstprivate(star_index)
        {
            int smallest_sum_priv     = std::numeric_limits<int>::max();
            int selected_interval_priv = -1;

            #pragma omp for
            for (int j = 0; j < INTERVALS; ++j) {
                if (array[star_index][j] == 1 &&
                    capacity[j] < MACHINES &&
                    column_sum[j] < smallest_sum_priv) {

                    smallest_sum_priv     = column_sum[j];
                    selected_interval_priv = j;
                }
            }

            #pragma omp critical
            {
                if (smallest_sum_priv < smallest_sum) {
                    smallest_sum     = smallest_sum_priv;
                    selected_interval = selected_interval_priv;
                }
            }
        }

        if (selected_interval != -1) {
            schedule[selected_interval].push_back(star_index);
            cout << "Star " << star_index ;
            if(VAR_SCORE) 
                cout << " with score " << scores[star_index];
            cout << " scheduled at interval " << selected_interval ;
            if(MACHINES > 1)
                cout << " at machine " << capacity[selected_interval];
            cout << endl;
            capacity[selected_interval]++;
            pts += scores[star_index];
        }
    }

    cout << "\nFinal Schedule:\n";
    for (int i = 0; i < INTERVALS; ++i) {
        cout << "Interval " << i << ": ";
        for (int star : schedule[i]) {
            cout << star << " ";
        }
        cout << "\n";
    }

    cout << "Total stars observed: " << pts << "\n";

    return 0;
}
