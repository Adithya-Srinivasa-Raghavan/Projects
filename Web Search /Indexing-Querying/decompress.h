#ifndef DECOMPRESS_H
#define DECOMPRESS_H

#include <string>
#include <vector>
#include <unordered_map>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>

struct PostingList {
    int term_id;
    std::string term;
    std::vector<int> doc_ids;
    std::vector<int> frequencies;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & term_id;
        ar & term;
        ar & doc_ids;
        ar & frequencies;
    }
};

// Declaration of functions
std::unordered_map<std::string, PostingList> decompressForQuery(
    const std::vector<std::string>& query_terms,
    const std::string& lexicon_file,
    const std::string& compressed_file
);

#endif // DECOMPRESS_H
