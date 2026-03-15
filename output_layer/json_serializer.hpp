#pragma once

#include <string>

#include "../shared_layer/types.hpp"

namespace teknegram {

class JsonSerializer {
    public:
        std::string serialize(const CorpusHierarchy& hierarchy) const;
};

} // namespace teknegram
