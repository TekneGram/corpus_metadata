#pragma once

#include <string>
#include <vector>

#include "../shared_layer/types.hpp"

namespace teknegram {

class DocumentCounter {
    public:
        std::vector<DocumentStats> count(const std::string& artifacts_dir) const;
};

} // namespace teknegram
