#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <numeric>
#include <limits>

using namespace std;

const int MACHINES = 5;
const int STARS = 110;
const int RANGE = 448;
const int OBSERVATION_TIME = 10;
const int SWITCH_TIME = 5;
const int TOTAL_TIME = OBSERVATION_TIME + SWITCH_TIME;
const int INTERVALS = RANGE / TOTAL_TIME;

vector<vector<int>> csvData;  // renamed from data

// Read CSV excluding first line and first column, parse integers
void read_csv(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        exit(1);
    }

    string line;
    bool first_line = true;
    while (getline(file, line)) {
        if (first_line) {
            first_line = false;
            continue;  // skip header line
        }
        stringstream ss(line);
        string cell;
        vector<int> row;
        bool skip_first_col = true;
        while (getline(ss, cell, ',')) {
            if (skip_first_col) {
                skip_first_col = false;
                continue;
            }
            try {
                row.push_back(stoi(cell));
            } catch (...) {
                // If conversion fails, push -1 as default error value
                row.push_back(-1);
            }
        }
        csvData.push_back(row);
    }
}

// Corrected valid function logic
bool valid(int i, int j) {
    // Check boundaries
    if (i < 0 || i >= (int)csvData.size()) return false;
    if (csvData[i].size() < 3) return false;

    int start = csvData[i][0];
    int end = csvData[i][1];
    int mid = csvData[i][2];

    if (start == -1) return false;

    int interval_start = j * TOTAL_TIME;
    int interval_end = (j + 1) * TOTAL_TIME;

    if (interval_start >= start && interval_end <= end) {
        if (mid == -1 ||
            (interval_end <= mid) ||
            (interval_start >= mid)) {
            return true;
        }
    }

    return false;
}

int main() {
    read_csv("data.csv");

    // Initialize random number generator for scores 1 to 5
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(1, 5);

    vector<int> scores(STARS);
    for (int& score : scores) {
        score = dist(gen);
    }

    // Initialize array with zeros
    vector<vector<int>> array(STARS, vector<int>(INTERVALS, 0));
    for (int i = 0; i < STARS; ++i) {
        for (int j = 0; j < INTERVALS; ++j) {
            if (valid(i, j)) {
                array[i][j] = 1;
            }
        }
    }

    // Create sorted indices of stars by descending scores
    vector<int> sorted_stars(STARS);
    iota(sorted_stars.begin(), sorted_stars.end(), 0);
    sort(sorted_stars.begin(), sorted_stars.end(),
         [&scores](int a, int b) { return scores[a] > scores[b]; });

    // Compute column sums (sum of each interval)
    vector<int> column_sum(INTERVALS, 0);
    for (int j = 0; j < INTERVALS; ++j) {
        int sum = 0;
        for (int i = 0; i < STARS; ++i) {
            sum += array[i][j];
        }
        column_sum[j] = sum;
    }

    // Prepare schedule and capacity tracking
    vector<vector<int>> schedule(INTERVALS);
    vector<int> capacity(INTERVALS, 0);
    int pts = 0;

    // Scheduling logic
    for (int i = 0; i < STARS; ++i) {
        int star_idx = sorted_stars[i];
        int smallest_sum = numeric_limits<int>::max();
        int selected_interval = -1;

        for (int j = 0; j < INTERVALS; ++j) {
            if (array[star_idx][j] == 1 && column_sum[j] < smallest_sum && capacity[j] < MACHINES) {
                smallest_sum = column_sum[j];
                selected_interval = j;
            }
        }

        if (selected_interval != -1) {
            schedule[selected_interval].push_back(star_idx);
            cout << "Star " << star_idx << " with score " << scores[star_idx]
                 << " scheduled at interval " << selected_interval
                 << " at machine " << capacity[selected_interval] << "\n";
            capacity[selected_interval]++;
            pts += scores[star_idx];
        }
    }

    for (int i = 0; i < INTERVALS; ++i) {
        cout << "Interval " << i << ":";
        for (int star : schedule[i]) {
            cout << " " << star;
        }
        cout << "\n";
    }

    cout << "Total points: " << pts << "\n";

    return 0;
}
