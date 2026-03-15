#include "corpus_metadata_engine.hpp"

#include <stdexcept>

#include "../aggregation_layer/subcorpus_aggregator.hpp"
#include "../counting_layer/document_counter.hpp"
#include "../metadata_layer/corpus_hierarchy_loader.hpp"
#include "../output_layer/json_serializer.hpp"

namespace teknegram {

std::string CorpusMetadataEngine::run(const std::string& corpus_name,
                                      const std::string& artifacts_dir,
                                      const ProgressEmitter* progress_emitter) const {
    if (corpus_name.empty()) {
        throw std::runtime_error("Corpus name is required.");
    }
    if (artifacts_dir.empty()) {
        throw std::runtime_error("Artifacts directory is required.");
    }

    NullProgressEmitter default_emitter;
    const ProgressEmitter* emitter = progress_emitter ? progress_emitter : &default_emitter;

    emitter->emit("Loading corpus hierarchy", 10);
    CorpusHierarchyLoader hierarchy_loader;
    CorpusHierarchy hierarchy = hierarchy_loader.load(artifacts_dir + "/corpus.db", corpus_name);

    emitter->emit("Counting document statistics", 45);
    DocumentCounter counter;
    const std::vector<DocumentStats> document_stats = counter.count(artifacts_dir);

    emitter->emit("Aggregating subtree statistics", 80);
    SubcorpusAggregator aggregator;
    aggregator.aggregate(&hierarchy, document_stats);

    emitter->emit("Serializing metadata JSON", 95);
    JsonSerializer serializer;
    const std::string output = serializer.serialize(hierarchy);

    emitter->emit("Corpus metadata complete", 100);
    return output;
}

} // namespace teknegram
