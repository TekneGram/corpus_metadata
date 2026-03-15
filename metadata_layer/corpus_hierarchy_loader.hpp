#pragma once

#include <string>

#include "../shared_layer/types.hpp"

namespace teknegram {

class CorpusHierarchyLoader {
    public:
        CorpusHierarchy load(const std::string& db_path,
                             const std::string& corpus_name) const;
};

} // namespace teknegram
