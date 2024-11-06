#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <sstream>
#include <algorithm>
#include <vector>
#include <regex>

using namespace std;

int main() {
    string outputDir = "/Users/adithya/Desktop/Web-Search-Engines-Thrus/Assignment-2/testing/"; // Output directory
    ifstream file("/Users/adithya/Desktop/Web-Search-Engines-Thrus/Assignment-2/collection.tsv");
    if (!file.is_open()) {
        cerr << "Error: Could not open the file." << endl;
        return 1;
    }

    int line_count = 0;
    int file_count = 1;
    map<string, vector<pair<int, int>>> inverted_index; // Word -> List of (DocID, Frequency) pairs

    // Updated regex to match words that contain only letters (ignores numbers)
    regex word_regex("^[a-zA-Z]+$");
    string line;

    // Process each line in the file
    while (getline(file, line)) {
        line_count++;
        stringstream ss(line);
        string word;
        int docID = line_count; // Use line count as DocID directly

        // Split line into words
        while (ss >> word) {
            // Remove non-alphanumeric characters and make lowercase
            word.erase(remove_if(word.begin(), word.end(), [](unsigned char c) {
                return !(isalnum(c) || c == '\''); 
            }), word.end());
            transform(word.begin(), word.end(), word.begin(), ::tolower);

            // Skip words that do not match the regex (e.g., numbers and mixed content)
            if (!regex_match(word, word_regex)) {
                continue;
            }

            bool found = false;
            for (auto& pair : inverted_index[word]) {
                if (pair.first == docID) {
                    pair.second++;
                    found = true;
                    break;
                }
            }
            if (!found) {
                inverted_index[word].push_back(make_pair(docID, 1));
            }
        }

        // Write the inverted index to a file every 10000 lines
        if (line_count % 10000 == 0) {
            ofstream output_file(outputDir + "inverted_index_" + to_string(file_count) + ".txt");
            if (!output_file.is_open()) {
                cerr << "Error: Could not open output file." << endl;
                return 1;
            }

            for (const auto& word_entry : inverted_index) {
                output_file << word_entry.first << ": ";
                for (size_t i = 0; i < word_entry.second.size(); ++i) {
                    output_file << "(" << word_entry.second[i].first << ", " << word_entry.second[i].second << ")";
                    if (i != word_entry.second.size() - 1) output_file << ", ";
                }
                output_file << endl;
            }

            output_file.close();
            file_count++;
            inverted_index.clear(); // Clear the map for the next set
        }
    }

    file.close();

    // Write any remaining lines to the last file if needed
    if (!inverted_index.empty()) {
        ofstream output_file(outputDir + "inverted_index_" + to_string(file_count) + ".txt");
        if (!output_file.is_open()) {
            cerr << "Error: Could not open output file." << endl;
            return 1;
        }

        for (const auto& word_entry : inverted_index) {
            output_file << word_entry.first << ": ";
            for (size_t i = 0; i < word_entry.second.size(); ++i) {
                output_file << "(" << word_entry.second[i].first << ", " << word_entry.second[i].second << ")";
                if (i != word_entry.second.size() - 1) output_file << ", ";
            }
            output_file << endl;
        }

        output_file.close();
    }

    cout << "Inverted index created successfully." << endl;
    return 0;
}
