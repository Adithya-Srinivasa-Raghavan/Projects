#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>

using namespace std;

struct PostingList {
    int term_id;
    string term;
    vector<int> doc_ids;
    vector<int> frequencies;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version) {
        ar & term_id;
        ar & term;
        ar & doc_ids;
        ar & frequencies;
    }
};

void compressIndex(const string& inputFile, const string& outputFile) {
    ifstream in(inputFile);
    ofstream out(outputFile, ios::binary);
    boost::archive::binary_oarchive archive(out);

    string line;
    while (getline(in, line)) {
        stringstream ss(line);
        PostingList posting;
        ss >> posting.term_id >> posting.term;
        char discard;

        int doc_id, freq;
        bool parsing_doc_ids = true;

        while (ss >> discard) {
            if (discard == '(') continue;
            if (discard == ')') {
                parsing_doc_ids = false;
                continue;
            }

            ss.putback(discard);
            if (parsing_doc_ids) {
                ss >> doc_id;
                posting.doc_ids.push_back(doc_id);
            } else {
                ss >> freq;
                posting.frequencies.push_back(freq);
            }
            if (ss.peek() == ',') ss.ignore();
        }

        archive << posting;
    }

    in.close();
    out.close();
    cout << "Compression complete. Data written to " << outputFile << endl;
}

int main() {
    string input_file = "/Users/adithya/Desktop/Web-Search-Engines-Thrus/Assignment-2/testing/merged_inverted_index.txt";
    string output_file = "/Users/adithya/Desktop/Web-Search-Engines-Thrus/Assignment-2/testing/compressed_inverted_index.bin";
    compressIndex(input_file, output_file);
    return 0;
}
