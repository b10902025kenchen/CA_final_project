#ifndef WEIGHT_INT_SCH_H
#define WEIGHT_INT_SCH_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>

using namespace std;


// Check if interval [x,y] is valid for star i according to data
bool valid(const vector<vector<int>>& data, int i, int x, int y) {
    if (data[i][0] != -1 && x >= data[i][0] && y <= data[i][1]) {
        if (data[i][2] == -1 || (x <= data[i][2] && y <= data[i][2]) || (x >= data[i][3] && y >= data[i][3])) {
            return true;
        }
    }
    return false;
}

// Find latest non conflicting task index for weighted interval scheduling
int latest_non_conflicting_task(const vector<tuple<int,int,double,int>>& tasks, int i) {
    for (int j = i - 1; j >= 0; j--) {
        if (get<1>(tasks[j]) <= get<0>(tasks[i])) {
            return j;
        }
    }
    return -1;
}

double weighted_interval_scheduling(const vector<int>& start_time, const vector<int>& end_time, const vector<double>& scores, int machines) {
    vector<tuple<int,int,double,int>> tasks;

    for (int i = 0; i < (int)start_time.size(); ++i) {
        if (start_time[i] != -1) {
            tasks.emplace_back(start_time[i], end_time[i], scores[i], i);
        }
    }

    // Sort by end time
    sort(tasks.begin(), tasks.end(), [](auto &a, auto &b) {
        return get<1>(a) < get<1>(b);
    });

    vector<double> dp(tasks.size(), 0.0);
    vector<int> traceback(tasks.size(), -1);
    vector<vector<tuple<int,int,double,int>>> selected_tasks(machines);

    // Copy of tasks to mark used tasks as null (emulated with invalid index -1)
    vector<tuple<int,int,double,int>> tasks_copy = tasks;

    for (int m = 0; m < machines; ++m) {
        for (size_t i = 0; i < tasks_copy.size(); ++i) {
            if (get<3>(tasks_copy[i]) == -1) continue; // task marked used

            double incl_score = get<2>(tasks_copy[i]);
            int l = latest_non_conflicting_task(tasks_copy, i);

            if (l != -1) {
                incl_score += dp[l];
            }

            double prev_dp = (i > 0) ? dp[i-1] : 0.0;
            if (incl_score > prev_dp) {
                dp[i] = incl_score;
                traceback[i] = l;
            } else {
                dp[i] = prev_dp;
                traceback[i] = -1;
            }
        }

        // Traceback to find tasks selected for this machine
        int i = (int)tasks_copy.size() - 1;
        while (i >= 0) {
            double prev_dp = (i > 0) ? dp[i-1] : 0.0;
            double curr_val = get<2>(tasks_copy[i]) + ((traceback[i] != -1) ? dp[traceback[i]] : 0.0);
            if (get<3>(tasks_copy[i]) != -1 && curr_val > prev_dp) {
                selected_tasks[m].push_back(tasks_copy[i]);
                // Mark task as used
                tasks_copy[i] = make_tuple(-1, -1, 0.0, -1);
                i = traceback[i];
            } else {
                i--;
            }
        }
    }

    // Calculate total weight
    double total_weight = 0.0;
    for (const auto& machine_tasks : selected_tasks) {
        for (const auto& task : machine_tasks) {
            total_weight += get<2>(task);
        }
    }

    // Print selected tasks
    for (int m = 0; m < machines; ++m) {
        if (selected_tasks[m].empty()) continue;
        cout << "Machine " << (m+1) << ":\n";
        for (auto it = selected_tasks[m].rbegin(); it != selected_tasks[m].rend(); ++it) {
            cout << "Task Index: " << get<3>(*it)
                 << ", Start time: " << get<0>(*it)
                 << ", End time: " << get<1>(*it)
                 << ", Score: " << get<2>(*it) << "\n";
        }
    }

    return total_weight;
}



#endif