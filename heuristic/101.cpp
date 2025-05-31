#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <ctime>
#include <tuple>
#include <set>

using namespace std;

const int STARS = 110;
const int RANGE = 448;
const int OBSERVATION_TIME = 10;
const int SWITCH_TIME = 5;
const int TOTAL_TIME = OBSERVATION_TIME + SWITCH_TIME;
const int MACHINES = 3;

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
    srand(time(nullptr));

    vector<vector<int>> data = read_csv("data.csv");

    vector<int> observation(STARS);
    vector<int> total(STARS);

    for (int i = 0; i < STARS; ++i) {
        observation[i] = rand() % 16 + 5;  // Random int in [5, 20]
        total[i] = observation[i] + SWITCH_TIME;
    }

    vector<int> start_time(STARS, 0);
    vector<int> end_time(STARS, 0);

    int num = 0, invalid = 0;
    for (int i = 0; i < STARS; ++i) {
        if (data[i][0] == -1 || data[i][3] - data[i][2] >= RANGE - total[i]) {
            start_time[i] = -1;
            end_time[i] = -1;
            ++invalid;
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
            for (int t = 0; t < 100; ++t) {
                int tmp = rand() % (RANGE - total[i] + 1);
                if (valid(data, i, tmp, tmp + total[i])) {
                    start_time[i] = tmp;
                    end_time[i] = tmp + total[i];
                    break;
                }
            }
        }
    }

    cout << STARS - invalid << endl;
    for (int i = 0; i < STARS; ++i) {
        cout << start_time[i] << " " << end_time[i] << " " << total[i] << endl;
    }

    // Scheduling
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
    set<int> used_indexes;

    for (int m = 0; m < MACHINES; ++m) {
        int prev_end_time = -1;
        for (const auto& task : tasks) {
            int start = get<0>(task);
            int end = get<1>(task);
            int index = get<2>(task);
            if (start >= prev_end_time && used_indexes.find(index) == used_indexes.end()) {
                selected_tasks.push_back(task);
                used_indexes.insert(index);
                prev_end_time = end;
            }
        }
    }

    cout << "Selected tasks:" << endl;
    for (const auto& task : selected_tasks) {
        cout << "Task Index: " << get<2>(task) << ", Start time: " << get<0>(task) << ", End time: " << get<1>(task) << endl;
    }
    cout << "Number of tasks assigned: " << selected_tasks.size() << endl;

    return 0;
}
