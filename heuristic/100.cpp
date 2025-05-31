#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <random>
#include <algorithm>

using namespace std;

const int STARS = 110;
const int RANGE = 448;
const int OBSERVATION_TIME = 10;
const int SWITCH_TIME = 5;
const int TOTAL_TIME = OBSERVATION_TIME + SWITCH_TIME;

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
        stringstream ss(line);
        string cell;
        vector<int> row;
        getline(ss, cell, ','); // skip first column
        while (getline(ss, cell, ',')) {
            row.push_back(stoi(cell));
        }
        data.push_back(row);
    }
    return data;
}

bool valid(const vector<vector<int>>& data, int i, int x, int y) {
    if (data[i][0] != -1 && x >= data[i][0] && y <= data[i][1]) {
        if (data[i][2] == -1 || (x <= data[i][2] && y <= data[i][2]) || (x >= data[i][3] && y >= data[i][3])) {
            return true;
        }
    }
    return false;
}

int main() {
    string filename = "data.csv";
    vector<vector<int>> data = read_csv(filename);

    vector<double> observation = {
        30, 30, 30, 20, 20, 20, 20, 30, 30, 30, 30, 30, 20, 30, 30, 30, 30, 30, 30, 40,
        30, 20, 30, 20, 30, 30, 30, 30, 30, 30, 20, 30, 20, 20, 20, 30, 30, 30, 20, 40,
        20, 20, 40, 20, 20, 30, 20, 20, 30, 30, 30, 30, 30, 30, 30, 30, 30, 40, 40, 30,
        40, 30, 30, 30, 40, 30, 30, 30, 30, 30, 30, 40, 40, 40, 30, 60, 30, 30, 30, 30,
        30, 30, 30, 40, 40, 30, 30, 40, 40, 40, 60, 30, 30, 30, 40, 40, 40, 60, 40, 40,
        30, 40, 30, 30, 40, 30, 30, 60, 40, 30
    };

    vector<int> total(STARS);
    for (int i = 0; i < STARS; ++i) {
        total[i] = observation[i] + SWITCH_TIME;
    }

    vector<int> start_time(STARS, 0);
    vector<int> end_time(STARS, 0);

    int num = 0;
    random_device rd;
    mt19937 gen(rd());

    for (int i = 0; i < STARS; ++i) {
        if (data[i][0] == -1 || data[i][3] - data[i][2] >= RANGE - total[i]) {
            start_time[i] = -1;
            end_time[i] = -1;
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
            uniform_int_distribution<> dis(0, RANGE - total[i]);
            bool found = false;
            for (int t = 0; t < 10; ++t) {
                int tmp = dis(gen);
                if (valid(data, i, tmp, tmp + total[i])) {
                    start_time[i] = tmp;
                    end_time[i] = tmp + total[i];
                    found = true;
                    break;
                }
            }
            if (!found) {
                start_time[i] = -1;
                end_time[i] = -1;
            }
        }
    }

    for (int i = 0; i < STARS; ++i) {
        cout << start_time[i] << " " << end_time[i] << " " << total[i] << endl;
    }

    // Greedy interval scheduling
    vector<tuple<int, int, int>> tasks;
    for (int i = 0; i < STARS; ++i) {
        if (start_time[i] != -1) {
            tasks.emplace_back(start_time[i], end_time[i], i);
        }
    }

    sort(tasks.begin(), tasks.end(), [](const auto& a, const auto& b) {
        return get<1>(a) < get<1>(b);
    });

    vector<tuple<int, int, int>> selected_tasks;
    int prev_end_time = -1;

    for (const auto& task : tasks) {
        int start = get<0>(task);
        int end = get<1>(task);
        if (start >= prev_end_time) {
            selected_tasks.push_back(task);
            prev_end_time = end;
        }
    }

    cout << "The maximum number of tasks that can be scheduled is: " << selected_tasks.size() << endl;
    cout << "Selected tasks:" << endl;
    for (const auto& task : selected_tasks) {
        int start = get<0>(task);
        int end = get<1>(task);
        int index = get<2>(task);
        cout << "Task Index: " << index << ", Start time: " << start << ", End time: " << end << endl;
    }

    return 0;
}
