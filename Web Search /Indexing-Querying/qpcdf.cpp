#include "decompress.h"
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>

using namespace std;

//String conversion to lowercase
string toLower(const string &str) {
    string lower_str = str;
    for (auto &ch : lower_str) ch = tolower(ch);
    return lower_str;
}

//Get the document statistics
map<int, int> loadDocStatistics(const string& docStatsFile, double &avg_doc_length) {
    ifstream docStatsFileIn(docStatsFile);
    map<int, int> doc_lengths;

    if (!docStatsFileIn.is_open()) {
        cerr << "Failed to open document statistics file." << endl;
        return doc_lengths;
    }

    string line;
    while (getline(docStatsFileIn, line)) {
        if (line.rfind("AverageDocumentLength,", 0) == 0) {
            avg_doc_length = stod(line.substr(line.find(",") + 1));
        } else {
            int docID, docLength;
            sscanf(line.c_str(), "%d, %d", &docID, &docLength);
            doc_lengths[docID] = docLength;
        }
    }

    docStatsFileIn.close();
    return doc_lengths;
}

// Calculate the BM25 score for a document
double calculateBM25(int doc_id, const string& term, const PostingList& posting, 
                     const map<int, int>& doc_lengths, double avg_doc_length) {
    const int total_docs = doc_lengths.size();
    const double k1 = 1.5, b = 0.75;

    auto it = find(posting.doc_ids.begin(), posting.doc_ids.end(), doc_id);
    if (it == posting.doc_ids.end()) return 0.0;

    int term_frequency = posting.frequencies[distance(posting.doc_ids.begin(), it)];
    int doc_frequency = posting.doc_ids.size();
    int doc_length = doc_lengths.at(doc_id);

    double idf = log((total_docs - doc_frequency + 0.5) / (doc_frequency + 0.5) + 1.0);
    double tf = term_frequency * (k1 + 1) / (term_frequency + k1 * (1 - b + b * doc_length / avg_doc_length));

    return idf * tf;
}

// Doing conjunctive (AND) query with BM25 scoring
map<int, double> processConjunctiveBM25(const unordered_map<string, PostingList>& query_postings, 
                                        const map<int, int>& doc_lengths, double avg_doc_length) {
    unordered_set<int> common_docs;
    auto it = query_postings.begin();
    if (it != query_postings.end()) {
        common_docs.insert(it->second.doc_ids.begin(), it->second.doc_ids.end());
        ++it;
    }

    for (; it != query_postings.end(); ++it) {
        unordered_set<int> term_docs(it->second.doc_ids.begin(), it->second.doc_ids.end());
        unordered_set<int> intersection;
        for (int doc_id : common_docs) {
            if (term_docs.count(doc_id)) {
                intersection.insert(doc_id);
            }
        }
        common_docs = intersection;
    }

    map<int, double> doc_scores;
    for (int doc_id : common_docs) {
        double score = 0.0;
        for (const auto& [term, posting] : query_postings) {
            score += calculateBM25(doc_id, term, posting, doc_lengths, avg_doc_length);
        }
        doc_scores[doc_id] = score;
    }
    return doc_scores;
}

// Doing disjunctive (OR) query with BM25 scoring
map<int, double> processDisjunctiveBM25(const unordered_map<string, PostingList>& query_postings, 
                                        const map<int, int>& doc_lengths, double avg_doc_length) {
    map<int, double> doc_scores;
    for (const auto& [term, posting] : query_postings) {
        for (int i = 0; i < posting.doc_ids.size(); ++i) {
            int doc_id = posting.doc_ids[i];
            double bm25_score = calculateBM25(doc_id, term, posting, doc_lengths, avg_doc_length);
            doc_scores[doc_id] += bm25_score;
        }
    }
    return doc_scores;
}

// Query processing with BM25 scoring
void searchQuery(const string& query, const string& lexicon_file, const string& compressed_file, 
                 const string& mode, const map<int, int>& doc_lengths, double avg_doc_length) {
    istringstream ss(query);
    vector<string> query_terms;
    string term;
    while (ss >> term) {
        query_terms.push_back(toLower(term));
    }

    unordered_map<string, PostingList> query_postings = decompressForQuery(query_terms, lexicon_file, compressed_file);

    if (query_postings.empty()) {
        cout << "No postings found for the query terms." << endl;
        return;
    }

    map<int, double> doc_scores;
    if (mode == "conjunctive") {
        doc_scores = processConjunctiveBM25(query_postings, doc_lengths, avg_doc_length);
        if (doc_scores.empty()) {
            cout << "No documents contain all query terms (Conjunctive Mode)." << endl;
            return;
        }
        cout << "Documents containing all query terms (Conjunctive Mode):" << endl;
    } else if (mode == "disjunctive") {
        doc_scores = processDisjunctiveBM25(query_postings, doc_lengths, avg_doc_length);
        cout << "Documents containing any query terms (Disjunctive Mode):" << endl;
    } else {
        cout << "Invalid mode. Please choose either 'conjunctive' or 'disjunctive'." << endl;
        return;
    }

    vector<pair<int, double>> sorted_docs(doc_scores.begin(), doc_scores.end());
    sort(sorted_docs.begin(), sorted_docs.end(), [](const pair<int, double>& a, const pair<int, double>& b) {
        return b.second < a.second;
    });

    for (const auto& [doc_id, score] : sorted_docs) {
        cout << "Doc ID: " << doc_id << ", Score: " << score << ", Frequencies: { ";
        for (const auto& [term, posting] : query_postings) {
            auto it = find(posting.doc_ids.begin(), posting.doc_ids.end(), doc_id);
            if (it != posting.doc_ids.end()) {
                int index = distance(posting.doc_ids.begin(), it);
                cout << term << ": " << posting.frequencies[index] << " ";
            }
        }
        cout << "}" << endl;
    }
}

int main() {
    string lexicon_file = "/Users/adithya/Desktop/Web-Search-Engines-Thrus/Assignment-2/testing/lexicon.txt";
    string compressed_file = "/Users/adithya/Desktop/Web-Search-Engines-Thrus/Assignment-2/testing/compressed_inverted_index.bin";
    string docStatsFile = "/Users/adithya/Desktop/Web-Search-Engines-Thrus/Assignment-2/testing/doc_statistics.txt";

    double avg_doc_length = 0.0;
    map<int, int> doc_lengths = loadDocStatistics(docStatsFile, avg_doc_length);

    while (true) {
        string query, mode;
        cout << "Enter your query (or type 'exit' to quit): ";
        getline(cin, query);
        if (query == "exit") break;

        cout << "Choose search mode (conjunctive/disjunctive): ";
        getline(cin, mode);

        searchQuery(query, lexicon_file, compressed_file, mode, doc_lengths, avg_doc_length);
    }

    return 0;
}
