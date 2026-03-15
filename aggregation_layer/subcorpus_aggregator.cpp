#include "subcorpus_aggregator.hpp"

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace teknegram {

namespace {

std::vector<std::uint32_t> UnionSortedVectors(const std::vector<std::uint32_t>& lhs,
                                              const std::vector<std::uint32_t>& rhs) {
    std::vector<std::uint32_t> merged;
    merged.reserve(lhs.size() + rhs.size());

    std::size_t i = 0;
    std::size_t j = 0;
    while (i < lhs.size() && j < rhs.size()) {
        if (lhs[i] < rhs[j]) {
            merged.push_back(lhs[i++]);
        } else if (rhs[j] < lhs[i]) {
            merged.push_back(rhs[j++]);
        } else {
            merged.push_back(lhs[i]);
            ++i;
            ++j;
        }
    }

    while (i < lhs.size()) {
        merged.push_back(lhs[i++]);
    }
    while (j < rhs.size()) {
        merged.push_back(rhs[j++]);
    }
    return merged;
}

void AggregateNode(std::size_t node_index,
                   CorpusHierarchy* hierarchy,
                   const std::vector<DocumentStats>& document_stats) {
    HierarchyNode& node = hierarchy->nodes[node_index];

    node.docs = static_cast<std::uint64_t>(node.direct_document_ids.size());
    node.words = 0U;
    node.aggregated_word_ids.clear();
    node.aggregated_lemma_ids.clear();

    for (std::size_t i = 0; i < node.direct_document_ids.size(); ++i) {
        const std::uint32_t doc_id = node.direct_document_ids[i];
        if (doc_id >= document_stats.size()) {
            throw std::runtime_error("Hierarchy references a document ID missing from doc_ranges.bin.");
        }
        const DocumentStats& stats = document_stats[doc_id];
        node.words += stats.word_tokens;
        node.aggregated_word_ids = UnionSortedVectors(node.aggregated_word_ids, stats.unique_word_ids);
        node.aggregated_lemma_ids = UnionSortedVectors(node.aggregated_lemma_ids, stats.unique_lemma_ids);
    }

    for (std::size_t i = 0; i < node.child_indices.size(); ++i) {
        const std::size_t child_index = node.child_indices[i];
        AggregateNode(child_index, hierarchy, document_stats);
        const HierarchyNode& child = hierarchy->nodes[child_index];
        node.docs += child.docs;
        node.words += child.words;
        node.aggregated_word_ids = UnionSortedVectors(node.aggregated_word_ids, child.aggregated_word_ids);
        node.aggregated_lemma_ids = UnionSortedVectors(node.aggregated_lemma_ids, child.aggregated_lemma_ids);
    }

    node.types = static_cast<std::uint64_t>(node.aggregated_word_ids.size());
    node.lemmas = static_cast<std::uint64_t>(node.aggregated_lemma_ids.size());
}

} // namespace

void SubcorpusAggregator::aggregate(CorpusHierarchy* hierarchy,
                                    const std::vector<DocumentStats>& document_stats) const {
    if (!hierarchy) {
        throw std::runtime_error("SubcorpusAggregator requires a hierarchy.");
    }
    AggregateNode(hierarchy->root_index, hierarchy, document_stats);
}

} // namespace teknegram
