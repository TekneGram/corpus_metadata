#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace teknegram {

static const std::uint8_t kPosPunctId = 13U;

struct MetadataRequest {
    std::string corpus_name;
    std::string artifacts_dir;
};

struct DocumentPath {
    std::uint32_t document_id;
    std::vector<std::string> segments;
};

struct DocumentStats {
    std::uint32_t document_id;
    std::uint64_t word_tokens;
    std::vector<std::uint32_t> unique_word_ids;
    std::vector<std::uint32_t> unique_lemma_ids;
};

struct HierarchyNode {
    explicit HierarchyNode(const std::string& node_name = std::string())
        : name(node_name), docs(0), words(0), lemmas(0), types(0) {}

    std::string name;
    std::vector<std::size_t> child_indices;
    std::vector<std::uint32_t> direct_document_ids;

    std::uint64_t docs;
    std::uint64_t words;
    std::uint64_t lemmas;
    std::uint64_t types;

    std::vector<std::uint32_t> aggregated_word_ids;
    std::vector<std::uint32_t> aggregated_lemma_ids;
};

struct CorpusHierarchy {
    explicit CorpusHierarchy(const std::string& corpus_name = std::string())
        : root_index(0U) {
        nodes.push_back(HierarchyNode(corpus_name));
    }

    std::vector<HierarchyNode> nodes;
    std::size_t root_index;
};

} // namespace teknegram
