#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <limits>
#include <algorithm>

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
            if (col_index > 0) { // skip the first column
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

    vector<vector<int>> array(STARS, vector<int>(INTERVALS, 0));
    for (int i = 0; i < STARS; ++i) {
        for (int j = 0; j < INTERVALS; ++j) {
            if (valid(data, i, j)) {
                array[i][j] = 1;
            }
        }
    }

    // Column sums
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
    vector<int> schedule(INTERVALS, -1);
    int observed_count = 0;

    for (int i = 0; i < STARS; ++i) {
        int smallest_sum = numeric_limits<int>::max();
        int selected_interval = -1;

        for (int j = 0; j < INTERVALS; ++j) {
            if (array[i][j] == 1 && column_sum[j] < smallest_sum && schedule[j] == -1) {
                smallest_sum = column_sum[j];
                selected_interval = j;
            }
        }

        if (selected_interval != -1) {
            schedule[selected_interval] = i;
            ++observed_count;
            cout << "Star " << i << " scheduled at interval " << selected_interval << "\n";
        }
    }

    cout << "\nFinal Schedule:\n";
    for (int j = 0; j < INTERVALS; ++j) {
        cout << schedule[j] << " ";
    }
    cout << "\n";

    cout << "Total stars observed: " << observed_count << "\n";

    return 0;
}
