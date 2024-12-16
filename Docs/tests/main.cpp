#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

#if defined(_WIN32) || defined(_WIN64)
#include <direct.h> // For Windows (_getcwd)
#define GetCWD _getcwd
#else
#include <unistd.h> // For Linux/macOS (getcwd)
#define GetCWD getcwd
#endif

using namespace std;

// Function to read nhmac values from the file
vector<string> readNHMACValues(const string &filename) {
    vector<string> nhmacValues;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Failed to open file " << filename << endl;

        char current_path[FILENAME_MAX];
        if (GetCWD(current_path, sizeof(current_path))) {
            cout << "Current working directory: " << current_path << endl;
        } else {
            cerr << "Error: Unable to determine the current working directory." << endl;
        }
        return nhmacValues;
    }

    string line;
    while (getline(file, line)) {
        // Look for the line starting with "nhmac:"
        if (line.rfind("nhmac:", 0) == 0) { // Check if the line starts with "nhmac:"
            string nhmac = line.substr(6); // Extract the part after "nhmac:"
            nhmac.erase(remove_if(nhmac.begin(), nhmac.end(), ::isspace), nhmac.end()); // Remove any whitespace
            nhmacValues.push_back(nhmac);
        }
    }

    file.close();
    return nhmacValues;
}

// Function to calculate median using lexicographical order
string calculateMedian(vector<string> &yValues) {
    sort(yValues.begin(), yValues.end()); // Sort lexicographically

    size_t n = yValues.size();
    if (n % 2 == 0) {
        return yValues[n / 2 - 1]; // For even n, return the lower middle value
    } else {
        return yValues[n / 2]; // For odd n, return the middle value
    }
}

// Function to generate the symbol sequence (a, b) based on the median
vector<char> generateSymbolSequence(vector<pair<string, string>> &keyPairs, const string &median) {
    vector<char> symbols;

    for (const auto &pair : keyPairs) {
        const string &yValue = pair.second;
        if (yValue < median) {
            symbols.push_back('a');
        } else if (yValue > median) {
            symbols.push_back('b');
        }
    }

    return symbols;
}

// Function to sort pairs by x-values
void sortPairsByX(vector<pair<string, string>> &keyPairs) {
    sort(keyPairs.begin(), keyPairs.end(), [](const pair<string, string> &a, const pair<string, string> &b) {
        return a.first < b.first;
    });
}

// Function to count the number of series in the symbol sequence
int countSeries(const vector<char> &symbols) {
    if (symbols.empty()) return 0;

    int seriesCount = 1; // Start with one series
    for (size_t i = 1; i < symbols.size(); ++i) {
        if (symbols[i] != symbols[i - 1]) {
            seriesCount++;
        }
    }
    return seriesCount;
}

// Calculation of critical series value m_alpha
int calculateCriticalSeries(int n, double alpha) {
    double criticalValue;
    if (alpha == 0.05) {
        criticalValue = (n + 1) / 2.0 - 0.82 * sqrt(n - 1);
    } else if (alpha == 0.01) {
        criticalValue = (n + 1) / 2.0 - 1.16 * sqrt(n - 1);
    } else {
        cerr << "Unsupported alpha = " << alpha << ". Defaulting to 0.05." << endl;
        criticalValue = (n + 1) / 2.0 - 0.82 * sqrt(n - 1);
    }
    return static_cast<int>(round(criticalValue)); // Round to nearest integer
}

// Nelson's Criterion
int nelsonTest(const vector<pair<string, string>> &keyPairs, int &n) {
    vector<int> signs;
    for (size_t i = 1; i < keyPairs.size(); ++i) {
        int deltaX = keyPairs[i].first.compare(keyPairs[i - 1].first);
        int deltaY = keyPairs[i].second.compare(keyPairs[i - 1].second);

        // Determine the sign of the pair
        if ((deltaX > 0 && deltaY > 0) || (deltaX < 0 && deltaY < 0)) {
            signs.push_back(1); // "+"
        } else if ((deltaX > 0 && deltaY < 0) || (deltaX < 0 && deltaY > 0)) {
            signs.push_back(-1); // "-"
        } else {
            signs.push_back(0); // "0"
        }
    }

    // Count the number of "+" and "-"
    int plusCount = count(signs.begin(), signs.end(), 1);
    int minusCount = count(signs.begin(), signs.end(), -1);

    // n - total number of "+" and "-"
    n = plusCount + minusCount;

    // Return the minimum of "+" and "-"
    return min(plusCount, minusCount);
}

// Function to write results to a CSV file
void writeResultsToCSV(const string &filename, const string &testFile, int n, int nelsonS, double criticalSAlpha,
                       int series, int criticalM, bool independenceS, bool independenceM) {
    ofstream file(filename, ios::app); // Append mode
    if (!file.is_open()) {
        cerr << "Error: Unable to open file " << filename << " for writing results." << endl;
        return;
    }

    // Write headers if the file is empty
    static bool headerWritten = false;
    if (!headerWritten) {
        file << "TestFile,n,NelsonS,CriticalSAlpha,Series,CriticalM,NelsonIndependence,SeriesIndependence\n";
        headerWritten = true;
    }

    // Write test data
    file << testFile << "," << n << "," << nelsonS << "," << criticalSAlpha << ","
         << series << "," << criticalM << "," << (independenceS ? "1" : "0") << ","
         << (independenceM ? "1" : "0") << "\n";

    file.close();
}


// Main function to perform the tests
void performTests(const string &filename, double alpha) {
    auto nhmacValues = readNHMACValues(filename);
    if (nhmacValues.size() < 2) {
        cerr << "Insufficient data for analysis in file: " << filename << endl;
        return;
    }

    // Convert nhmacValues to key pairs
    vector<pair<string, string>> keyPairs;
    for (size_t i = 0; i < nhmacValues.size() - 1; ++i) {
        keyPairs.emplace_back(nhmacValues[i], nhmacValues[i + 1]);
    }

    // Nelson's Test
    int n = 0;
    int nelsonS = nelsonTest(keyPairs, n);
    double criticalSAlpha = n / 2.0 + 1.645 * sqrt((11.0 * n - 2.0) / 36.0);
    bool independenceS = nelsonS <= criticalSAlpha;

    // Swed-Eisenhart Serial Criterion
    vector<string> yValues;
    for (const auto &pair : keyPairs) {
        yValues.push_back(pair.second);
    }

    string median = calculateMedian(yValues);
    sortPairsByX(keyPairs);
    auto symbols = generateSymbolSequence(keyPairs, median);
    int series = countSeries(symbols);
    int criticalM = calculateCriticalSeries(n, alpha);
    bool independenceM = series <= criticalM ;

    // Output results
    cout << "File: " << filename << endl;
    cout << "Sample size: " << n << " data points" << endl;
    cout << "Nelson's Criterion (S-statistic): " << nelsonS
         << " (critical value S_alpha = " << criticalSAlpha << ")" << endl;
    cout << "Nelson's result: " << (independenceS ? "Independence" : "Dependence") << endl;
    cout << "Swed-Eisenhart Serial Criterion (m): " << series
         << " (critical value m for alpha = " << alpha << ": " << criticalM << ")" << endl;
    cout << "Swed-Eisenhart result: " << (independenceM ? "Dependence" : "Independence") << endl;
    cout << endl;

    // Write results to CSV
    writeResultsToCSV("results.csv", filename, n, nelsonS, criticalSAlpha, series, criticalM, independenceS, independenceM);
}

int main() {
    vector<string> files = {
        "test_results_100_0.txt",
        "test_results_200_0.txt",
        "test_results_300_0.txt",
        "test_results_400_0.txt",
        "test_results_500_0.txt",
        "test_results_600_0.txt",
        "test_results_700_0.txt",
        "test_results_800_0.txt",
        "test_results_900_0.txt",
    };

    double alpha = 0.05; // Significance level

    for (const auto &file : files) {
        performTests(file, alpha);
    }

    return 0;
}
