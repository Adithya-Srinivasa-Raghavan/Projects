#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <string>
#include <sstream>
#include <unordered_map>
#include <map>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

//Declaring a Heap to perform the Sorting and Merging efficiently 
struct HeapNode {
    string term;
    int fileIndex;
    string docInfo;

    bool operator>(const HeapNode &other) const {
        return term > other.term;
    }
};

void mergeFiles(const vector<string>& fileNames, const string& outputFileName, const string& lexiconFileName) {
    vector<ifstream> files(fileNames.size());
    priority_queue<HeapNode, vector<HeapNode>, greater<HeapNode>> minHeap;
    unordered_map<string, int> termIDMap;
    int termID = 1;
    map<string, pair<vector<int>, vector<int>>> termData;

    // Initializing/Setting up the Heap as a Min-Heap with the first term from each file
    for (int i = 0; i < fileNames.size(); i++) {
        files[i].open(fileNames[i]);
        if (files[i].is_open()) {
            string line;
            if (getline(files[i], line)) {
                stringstream ss(line);
                string term, docInfo;
                getline(ss, term, ':');
                getline(ss, docInfo);
                minHeap.push({term, i, docInfo});
            }
        } else {
            cerr << "Error: Could not open file " << fileNames[i] << endl;
            return;
        }
    }

    // Process terms in sorted order
    while (!minHeap.empty()) {
        HeapNode smallest = minHeap.top();
        minHeap.pop();

        if (termIDMap.find(smallest.term) == termIDMap.end()) {
            termIDMap[smallest.term] = termID++;
        }

        vector<int> docIDs, frequencies;
        size_t pos = 0;

        while ((pos = smallest.docInfo.find("(")) != string::npos) {
            size_t endPos = smallest.docInfo.find(")", pos);
            if (endPos == string::npos) {
                cerr << "Error: Mismatched parentheses in docInfo for term: " << smallest.term << endl;
                break;
            }
            string docEntry = smallest.docInfo.substr(pos + 1, endPos - pos - 1);
            size_t commaPos = docEntry.find(",");
            if (commaPos != string::npos) {
                try {
                    int docID = stoi(docEntry.substr(0, commaPos));
                    int freq = stoi(docEntry.substr(commaPos + 1));
                    docIDs.push_back(docID);
                    frequencies.push_back(freq);
                } catch (const invalid_argument& e) {
                    cerr << "Error: Invalid DocID or Frequency in entry: " << docEntry << endl;
                }
            }
            smallest.docInfo = smallest.docInfo.substr(endPos + 1);
        }

        if (!docIDs.empty() && !frequencies.empty()) {
            if (termData.find(smallest.term) == termData.end()) {
                termData[smallest.term] = {docIDs, frequencies};
            } else {
                termData[smallest.term].first.insert(termData[smallest.term].first.end(), docIDs.begin(), docIDs.end());
                termData[smallest.term].second.insert(termData[smallest.term].second.end(), frequencies.begin(), frequencies.end());
            }
        }

        int fileIndex = smallest.fileIndex;
        string line;
        if (getline(files[fileIndex], line)) {
            stringstream ss(line);
            string term, docInfo;
            getline(ss, term, ':');
            getline(ss, docInfo);
            minHeap.push({term, fileIndex, docInfo});
        }
    }

    for (auto& file : files) {
        if (file.is_open()) {
            file.close();
        }
    }

    ofstream outputFile(outputFileName);
    if (!outputFile.is_open()) {
        cerr << "Error: Could not open output file." << endl;
        return;
    }

    for (const auto& entry : termData) {
        const string& term = entry.first;
        const auto& docIDs = entry.second.first;
        const auto& frequencies = entry.second.second;
        int currentTermID = termIDMap[term];

        outputFile << currentTermID << " " << term << " (";
        for (size_t i = 0; i < docIDs.size(); ++i) {
            outputFile << docIDs[i];
            if (i != docIDs.size() - 1) outputFile << ",";
        }
        outputFile << ") (";
        for (size_t i = 0; i < frequencies.size(); ++i) {
            outputFile << frequencies[i];
            if (i != frequencies.size() - 1) outputFile << ",";
        }
        outputFile << ")" << endl;
    }

    outputFile.close();

    ofstream lexiconFile(lexiconFileName);
    if (!lexiconFile.is_open()) {
        cerr << "Error: Could not open lexicon file." << endl;
        return;
    }

    for (const auto& entry : termIDMap) {
        lexiconFile << "Term:" << entry.first << " "
                    << "TermID:" << entry.second << " "
                    << "DocCount:" << termData[entry.first].first.size() << endl;
    }

    lexiconFile.close();
    cout << "Merged file and lexicon created successfully." << endl;
}

int main() {
    string inputDir = "/Users/adithya/Desktop/Web-Search-Engines-Thrus/Assignment-2/testing/";
    string outputFileName = "/Users/adithya/Desktop/Web-Search-Engines-Thrus/Assignment-2/testing/merged_inverted_index.txt";
    string lexiconFileName = "/Users/adithya/Desktop/Web-Search-Engines-Thrus/Assignment-2/testing/lexicon.txt";
    vector<string> fileNames;

    for (const auto& entry : fs::directory_iterator(inputDir)) {
        if (entry.path().extension() == ".txt" && entry.path().filename().string().find("inverted_index_") == 0) {
            fileNames.push_back(entry.path().string());
        }
    }

    if (!fileNames.empty()) {
        mergeFiles(fileNames, outputFileName, lexiconFileName);
    } else {
        cerr << "No files found in the specified directory." << endl;
    }

    return 0;
}
