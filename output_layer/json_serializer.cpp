#include "json_serializer.hpp"

#include "../../corpus_builder/third_party/nlohmann/json.hpp"

namespace teknegram {

namespace {

using json = nlohmann::json;

json SerializeNode(const CorpusHierarchy& hierarchy, std::size_t node_index) {
    const HierarchyNode& node = hierarchy.nodes[node_index];
    json payload = json::object();
    payload["name"] = node.name;
    payload["docs"] = node.docs;
    payload["words"] = node.words;
    payload["lemmas"] = node.lemmas;
    payload["types"] = node.types;
    payload["subcorpora"] = json::array();

    for (std::size_t i = 0; i < node.child_indices.size(); ++i) {
        payload["subcorpora"].push_back(SerializeNode(hierarchy, node.child_indices[i]));
    }
    return payload;
}

} // namespace

std::string JsonSerializer::serialize(const CorpusHierarchy& hierarchy) const {
    const HierarchyNode& root = hierarchy.nodes[hierarchy.root_index];

    json payload = json::object();
    payload["corpus_name"] = root.name;
    payload["docs"] = root.docs;
    payload["words"] = root.words;
    payload["lemmas"] = root.lemmas;
    payload["types"] = root.types;
    payload["subcorpora"] = json::array();

    for (std::size_t i = 0; i < root.child_indices.size(); ++i) {
        payload["subcorpora"].push_back(SerializeNode(hierarchy, root.child_indices[i]));
    }
    return payload.dump();
}

} // namespace teknegram
