#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <random>
#include <algorithm>
#include <numeric>

using namespace std;

const int MACHINES = 3;
const int STARS = 110;
const int RANGE = 448;
const int OBSERVATION_TIME = 20;
const int SWITCH_TIME = 5;
const int TOTAL_TIME = OBSERVATION_TIME + SWITCH_TIME;
const int INTERVALS = (RANGE / (TOTAL_TIME));

vector<vector<int>> read_csv(const string &filename) {
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
        getline(ss, cell, ','); // Skip the first column
        while (getline(ss, cell, ',')) {
            row.push_back(stoi(cell));
        }
        data.push_back(row);
    }

    return data;
}

bool valid(const vector<vector<int>> &data, int i, int x, int y) {
    if (data[i][0] != -1 && x >= data[i][0] && y <= data[i][1]) {
        if (data[i][2] == -1 || (x <= data[i][2] && y <= data[i][2]) || (x >= data[i][3] && y >= data[i][3])) {
            return true;
        }
    }
    return false;
}

struct Task {
    int start, end, score, index;
};

int latest_non_conflicting(const vector<Task> &tasks, int i) {
    for (int j = i - 1; j >= 0; j--) {
        if (tasks[j].end <= tasks[i].start)
            return j;
    }
    return -1;
}

int weighted_interval_scheduling(const vector<int> &start_time, const vector<int> &end_time, const vector<double> &scores) {
    vector<Task> tasks;
    for (int i = 0; i < STARS; i++) {
        if (start_time[i] != -1) {
            tasks.push_back({start_time[i], end_time[i], (int)scores[i], i});
        }
    }

    sort(tasks.begin(), tasks.end(), [](const Task &a, const Task &b) {
        return a.end < b.end;
    });

    vector<int> dp(tasks.size(), 0);
    vector<int> traceback(tasks.size(), -1);

    for (int i = 0; i < tasks.size(); ++i) {
        int incl_score = tasks[i].score;
        int l = latest_non_conflicting(tasks, i);
        if (l != -1) incl_score += dp[l];

        if (i == 0 || incl_score > dp[i - 1]) {
            dp[i] = incl_score;
            traceback[i] = l;
        } else {
            dp[i] = dp[i - 1];
        }
    }

    vector<Task> selected;
    int i = tasks.size() - 1;
    while (i >= 0) {
        int incl_score = tasks[i].score;
        if (traceback[i] != -1) incl_score += dp[traceback[i]];

        if (i == 0 || incl_score > dp[i - 1]) {
            selected.push_back(tasks[i]);
            i = traceback[i];
        } else {
            i--;
        }
    }

    reverse(selected.begin(), selected.end());

    cout << "Selected tasks:\n";
    for (const auto &task : selected) {
        cout << "Task Index: " << task.index << ", Start time: " << task.start << ", End time: " << task.end << ", Score: " << task.score << endl;
    }

    int total_weight = 0;
    for (const auto &task : selected) total_weight += task.score;

    return total_weight;
}

int main() {
    vector<vector<int>> data = read_csv("data.csv");

    vector<double> observation = {
        30.0, 30.0, 30.0, 20.0, 20.0, 20.0, 20.0, 30.0, 30.0, 30.0, 30.0, 30.0, 20.0, 30.0, 30.0, 30.0, 30.0, 30.0, 30.0, 40.0,
        30.0, 20.0, 30.0, 20.0, 30.0, 30.0, 30.0, 30.0, 30.0, 30.0, 20.0, 30.0, 20.0, 20.0, 20.0, 30.0, 30.0, 30.0, 20.0, 40.0,
        20.0, 20.0, 40.0, 20.0, 20.0, 30.0, 20.0, 20.0, 30.0, 30.0, 30.0, 30.0, 30.0, 30.0, 30.0, 30.0, 30.0, 40.0, 40.0, 30.0,
        40.0, 30.0, 30.0, 30.0, 40.0, 30.0, 30.0, 30.0, 30.0, 30.0, 30.0, 40.0, 40.0, 40.0, 30.0, 60.0, 30.0, 30.0, 30.0, 30.0,
        30.0, 30.0, 30.0, 40.0, 40.0, 30.0, 30.0, 40.0, 40.0, 40.0, 60.0, 30.0, 30.0, 30.0, 40.0, 40.0, 40.0, 60.0, 40.0, 40.0,
        30.0, 40.0, 30.0, 30.0, 40.0, 30.0, 30.0, 60.0, 40.0, 30.0
    };

    vector<int> total(STARS);
    for (int i = 0; i < STARS; i++) total[i] = (int)observation[i] + SWITCH_TIME;

    vector<double> scores = {
        3.0, 2.0, 2.0, 4.0, 1.0, 2.0, 2.0, 3.0, 3.0, 3.0, 1.0, 3.0, 1.0, 3.0, 2.0, 2.0, 2.0, 3.0, 3.0, 5.0, 3.0, 1.0, 2.0, 3.0,
        2.0, 3.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 4.0, 2.0, 1.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 1.0, 2.0, 2.0, 1.0, 2.0, 2.0, 2.0,
        3.0, 2.0, 3.0, 2.0, 2.0, 3.0, 4.0, 3.0, 2.0, 3.0, 4.0, 3.0, 4.0, 3.0, 3.0, 3.0, 3.0, 2.0, 2.0, 5.0, 3.0, 3.0, 3.0, 3.0,
        4.0, 5.0, 3.0, 3.0, 3.0, 3.0, 3.0, 2.0, 2.0, 2.0, 4.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 4.0, 5.0, 2.0, 2.0, 3.0, 4.0, 3.0,
        5.0, 5.0, 4.0, 5.0, 5.0, 4.0, 2.0, 2.0, 3.0, 4.0, 4.0, 4.0, 4.0, 4.0
    };

    vector<int> start_time(STARS, -1);
    vector<int> end_time(STARS, -1);

    int num = 0, invalid = 0;
    random_device rd;
    mt19937 gen(rd());

    for (int i = 0; i < STARS; i++) {
        if (data[i][0] == -1 || data[i][3] - data[i][2] >= RANGE - total[i]) {
            start_time[i] = -1;
            end_time[i] = -1;
            invalid++;
        } else {
            if (num + total[i] >= RANGE) num = 0;
            if (valid(data, i, num, num + total[i])) {
                start_time[i] = num;
                end_time[i] = num + total[i];
                num = end_time[i];
            } else {
                uniform_int_distribution<> dis(0, RANGE - total[i]);
                for (int t = 0; t < 100; ++t) {
                    int tmp = dis(gen);
                    if (valid(data, i, tmp, tmp + total[i])) {
                        start_time[i] = tmp;
                        end_time[i] = tmp + total[i];
                        break;
                    }
                }
            }
        }
    }

    cout << STARS - invalid << endl;
    for (int i = 0; i < STARS; i++) {
        cout << start_time[i] << " " << end_time[i] << " " << total[i] << " " << scores[i] << endl;
    }

    int max_weight = weighted_interval_scheduling(start_time, end_time, scores);
    cout << "Maximum Weight: " << max_weight << endl;

    return 0;
}
