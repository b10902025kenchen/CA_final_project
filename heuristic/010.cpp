#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <algorithm>
#include <random>
#include <ctime>
#include <limits>

using namespace std;

const int STARS = 110;
const int RANGE = 448;
const int OBSERVATION_TIME = 10;
const int SWITCH_TIME = 5;
const int TOTAL_TIME = OBSERVATION_TIME + SWITCH_TIME;
const int INTERVALS = RANGE / TOTAL_TIME;

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
            if (col_index > 0) {
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

int main() {
    string filename = "data.csv";
    vector<vector<int>> data = read_csv(filename);

    // Random score generation
    vector<int> scores(STARS);
    mt19937 rng(time(0));
    uniform_int_distribution<int> dist(1, 5);
    for (int i = 0; i < STARS; ++i) {
        scores[i] = dist(rng);
    }

    // Print scores
    cout << "Scores:\n";
    for (int i = 0; i < STARS; ++i) {
        cout << scores[i] << " ";
    }
    cout << "\n";

    // Valid array
    vector<vector<int>> array(STARS, vector<int>(INTERVALS, 0));
    for (int i = 0; i < STARS; ++i) {
        for (int j = 0; j < INTERVALS; ++j) {
            if (valid(data, i, j)) {
                array[i][j] = 1;
            }
        }
    }

    // Sort stars by score (descending)
    vector<int> sorted_stars(STARS);
    for (int i = 0; i < STARS; ++i) sorted_stars[i] = i;

    sort(sorted_stars.begin(), sorted_stars.end(), [&](int a, int b) {
        return scores[a] > scores[b];
    });

    cout << "Sorted star indices:\n";
    for (int i : sorted_stars) {
        cout << i << " ";
    }
    cout << "\n";

    // Compute column sums
    vector<int> column_sum(INTERVALS, 0);
    for (int j = 0; j < INTERVALS; ++j) {
        for (int i = 0; i < STARS; ++i) {
            column_sum[j] += array[i][j];
        }
    }

    // Greedy scheduling
    vector<int> schedule(INTERVALS, -1);  // -1 means empty
    int pts = 0;

    for (int i = 0; i < STARS; ++i) {
        int star = sorted_stars[i];
        int smallest_sum = numeric_limits<int>::max();
        int selected_interval = -1;

        for (int j = 0; j < INTERVALS; ++j) {
            if (array[star][j] == 1 && column_sum[j] < smallest_sum && schedule[j] == -1) {
                smallest_sum = column_sum[j];
                selected_interval = j;
            }
        }

        if (selected_interval != -1) {
            schedule[selected_interval] = star;
            pts += scores[star];
            cout << "Star " << star << " with score " << scores[star] 
                 << " scheduled at interval " << selected_interval + 1 << "\n";
        }
    }

    cout << "\nSchedule:\n";
    for (int j = 0; j < INTERVALS; ++j) {
        cout << schedule[j] << " ";
    }
    cout << "\n";

    cout << "Total score: " << pts << "\n";

    return 0;
}
