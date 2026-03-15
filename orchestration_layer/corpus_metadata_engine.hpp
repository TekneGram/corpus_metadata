#pragma once

#include <string>

#include "../shared_layer/progress_emitter.hpp"

namespace teknegram {

class CorpusMetadataEngine {
    public:
        std::string run(const std::string& corpus_name,
                        const std::string& artifacts_dir,
                        const ProgressEmitter* progress_emitter = 0) const;
};

} // namespace teknegram
