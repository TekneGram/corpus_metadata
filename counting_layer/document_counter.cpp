#include "document_counter.hpp"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <unordered_set>
#include <utility>
#include <vector>

namespace teknegram {

namespace {

std::vector<std::pair<std::uint32_t, std::uint32_t> > LoadDocRanges(const std::string& path) {
    std::ifstream in(path.c_str(), std::ios::binary | std::ios::in);
    if (!in) {
        throw std::runtime_error("Failed to open doc_ranges.bin: " + path);
    }

    std::vector<std::pair<std::uint32_t, std::uint32_t> > ranges;
    std::uint32_t begin = 0;
    std::uint32_t end = 0;
    while (in.read(reinterpret_cast<char*>(&begin), sizeof(begin))) {
        if (!in.read(reinterpret_cast<char*>(&end), sizeof(end))) {
            throw std::runtime_error("Malformed doc_ranges.bin: odd uint32 count.");
        }
        ranges.push_back(std::make_pair(begin, end));
    }
    return ranges;
}

std::vector<std::uint32_t> SortedVectorFromSet(const std::unordered_set<std::uint32_t>& values) {
    std::vector<std::uint32_t> out;
    out.reserve(values.size());
    for (std::unordered_set<std::uint32_t>::const_iterator it = values.begin(); it != values.end(); ++it) {
        out.push_back(*it);
    }
    std::sort(out.begin(), out.end());
    return out;
}

} // namespace

std::vector<DocumentStats> DocumentCounter::count(const std::string& artifacts_dir) const {
    const std::vector<std::pair<std::uint32_t, std::uint32_t> > doc_ranges =
        LoadDocRanges(artifacts_dir + "/doc_ranges.bin");

    std::ifstream word_in((artifacts_dir + "/word.bin").c_str(), std::ios::binary | std::ios::in);
    std::ifstream lemma_in((artifacts_dir + "/lemma.bin").c_str(), std::ios::binary | std::ios::in);
    std::ifstream pos_in((artifacts_dir + "/pos.bin").c_str(), std::ios::binary | std::ios::in);
    if (!word_in || !lemma_in || !pos_in) {
        throw std::runtime_error("Failed to open one or more core token binaries.");
    }

    std::vector<DocumentStats> stats;
    stats.reserve(doc_ranges.size());

    for (std::size_t doc_index = 0; doc_index < doc_ranges.size(); ++doc_index) {
        const std::uint32_t doc_id = static_cast<std::uint32_t>(doc_index);
        const std::uint32_t doc_start = doc_ranges[doc_index].first;
        const std::uint32_t doc_end = doc_ranges[doc_index].second;
        if (doc_end < doc_start) {
            throw std::runtime_error("Malformed doc_ranges.bin: document end precedes start.");
        }

        DocumentStats doc_stats;
        doc_stats.document_id = doc_id;
        doc_stats.word_tokens = 0U;

        std::unordered_set<std::uint32_t> unique_word_ids;
        std::unordered_set<std::uint32_t> unique_lemma_ids;

        for (std::uint32_t token = doc_start; token < doc_end; ++token) {
            (void)token;
            std::uint32_t word_id = 0;
            std::uint32_t lemma_id = 0;
            std::uint8_t pos_id = 0;

            if (!word_in.read(reinterpret_cast<char*>(&word_id), sizeof(word_id)) ||
                !lemma_in.read(reinterpret_cast<char*>(&lemma_id), sizeof(lemma_id)) ||
                !pos_in.read(reinterpret_cast<char*>(&pos_id), sizeof(pos_id))) {
                throw std::runtime_error("Core token binaries ended before doc_ranges.bin.");
            }

            if (pos_id == kPosPunctId) {
                continue;
            }

            ++doc_stats.word_tokens;
            unique_word_ids.insert(word_id);
            unique_lemma_ids.insert(lemma_id);
        }

        doc_stats.unique_word_ids = SortedVectorFromSet(unique_word_ids);
        doc_stats.unique_lemma_ids = SortedVectorFromSet(unique_lemma_ids);
        stats.push_back(doc_stats);
    }

    return stats;
}

} // namespace teknegram
