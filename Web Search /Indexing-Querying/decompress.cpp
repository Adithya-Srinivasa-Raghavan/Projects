#include "decompress.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/archive/binary_iarchive.hpp>

std::unordered_map<std::string, PostingList> all_postings;

std::unordered_map<std::string, PostingList> decompressAll(const std::string& lexicon_file, const std::string& compressed_file) {
    std::ifstream lexicon_in(lexicon_file);
    if (!lexicon_in.is_open()) {
        std::cerr << "Failed to open lexicon file: " << lexicon_file << std::endl;
        return {};
    }

    std::ifstream compressed(compressed_file, std::ios::binary);
    if (!compressed.is_open()) {
        std::cerr << "Failed to open compressed file: " << compressed_file << std::endl;
        return {};
    }

    boost::archive::binary_iarchive archive(compressed);

    std::unordered_map<std::string, int> term_to_id;
    std::string line;

    // Map terms from lexicon
    while (getline(lexicon_in, line)) {
        std::string term;
        int term_id, doc_count;
        
        std::istringstream iss(line);
        std::string term_str, term_id_str, doc_count_str;

        iss >> term_str >> term_id_str >> doc_count_str;
        term = term_str.substr(term_str.find(":") + 1);
        
        try {
            term_id = std::stoi(term_id_str.substr(term_id_str.find(":") + 1));
            doc_count = std::stoi(doc_count_str.substr(doc_count_str.find(":") + 1));
        } catch (const std::exception& e) {
            std::cerr << "Error parsing term ID or doc count in line: " << line << std::endl;
            continue;
        }

        term_to_id[term] = term_id;
    }

    // Sequentially decompress and store all terms
    while (true) {
        PostingList decompressed_posting;
        try {
            archive >> decompressed_posting;
            for (const auto& [term, term_id] : term_to_id) {
                if (decompressed_posting.term_id == term_id) {
                    all_postings[term] = decompressed_posting;
                    break;
                }
            }
        } catch (const boost::archive::archive_exception& e) {
            std::cerr << "Reached end of archive or encountered error: " << e.what() << std::endl;
            break;
        }
    }

    return all_postings;
}

std::unordered_map<std::string, PostingList> decompressForQuery(
    const std::vector<std::string>& query_terms,
    const std::string& lexicon_file,
    const std::string& compressed_file
) {
    if (all_postings.empty()) {
        decompressAll(lexicon_file, compressed_file);
    }

    std::unordered_map<std::string, PostingList> query_postings;
    for (const std::string& term : query_terms) {
        if (all_postings.count(term)) {
            query_postings[term] = all_postings[term];
            std::cout << "Found term '" << term << "' with Term ID: " << all_postings[term].term_id << std::endl;
        } else {
            std::cout << "No postings found for term: " << term << std::endl;
        }
    }

    return query_postings;
}
