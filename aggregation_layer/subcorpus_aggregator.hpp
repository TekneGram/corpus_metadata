#pragma once

#include <vector>

#include "../shared_layer/types.hpp"

namespace teknegram {

class SubcorpusAggregator {
    public:
        void aggregate(CorpusHierarchy* hierarchy,
                       const std::vector<DocumentStats>& document_stats) const;
};

} // namespace teknegram
