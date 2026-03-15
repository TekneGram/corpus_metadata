#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

#include "../corpus_builder/third_party/nlohmann/json.hpp"
#include "orchestration_layer/corpus_metadata_engine.hpp"
#include "shared_layer/progress_emitter.hpp"

namespace {

using json = nlohmann::json;

const char* kInjectedCorpusName = "";
const char* kInjectedArtifactsDir = "";

void EmitJson(const json& payload);

class JsonProgressEmitter : public teknegram::ProgressEmitter {
    public:
        virtual void emit(const std::string& message, int percent) const {
            EmitJson(json{
                {"type", "progress"},
                {"message", message},
                {"percent", percent}
            });
        }
};

class CliProgressEmitter : public teknegram::ProgressEmitter {
    public:
        virtual void emit(const std::string& message, int percent) const {
            std::cerr << "[" << percent << "%] " << message << "\n";
            std::cerr.flush();
        }
};

bool HasNonWhitespace(const std::string& value) {
    for (std::string::size_type i = 0; i < value.size(); ++i) {
        const char ch = value[i];
        if (ch != ' ' && ch != '\n' && ch != '\r' && ch != '\t') {
            return true;
        }
    }
    return false;
}

std::string ReadStdin() {
    return std::string(
        std::istreambuf_iterator<char>(std::cin),
        std::istreambuf_iterator<char>()
    );
}

void EmitJson(const json& payload) {
    std::cout << payload.dump() << std::endl;
    std::cout.flush();
}

void EmitResult(const json& data) {
    EmitJson(json{
        {"type", "result"},
        {"data", data}
    });
}

void EmitError(const std::string& message, const std::string& code) {
    EmitJson(json{
        {"type", "error"},
        {"code", code},
        {"message", message}
    });
}

std::string RequireString(const json& input, const char* key) {
    if (!input.contains(key) || !input[key].is_string()) {
        throw std::runtime_error(std::string("Missing or invalid ") + key);
    }
    return input[key].get<std::string>();
}

std::string ResolveValue(const std::string& provided,
                         const char* injected,
                         const std::string& field_name) {
    if (!provided.empty()) {
        return provided;
    }
    if (injected && injected[0] != '\0') {
        return injected;
    }
    throw std::runtime_error(field_name + " is not configured.");
}

std::string RunExtraction(const std::string& corpus_name,
                          const std::string& artifacts_dir,
                          const teknegram::ProgressEmitter* emitter) {
    teknegram::CorpusMetadataEngine engine;
    return engine.run(corpus_name, artifacts_dir, emitter);
}

int RunCliMode(int argc, char** argv) {
    const std::string corpus_name = ResolveValue(
        argc > 1 ? argv[1] : std::string(),
        kInjectedCorpusName,
        "Corpus name");

    const std::string artifacts_dir = ResolveValue(
        argc > 2 ? argv[2] : std::string(),
        kInjectedArtifactsDir,
        "Artifacts directory");

    CliProgressEmitter progress_emitter;
    std::cout << RunExtraction(corpus_name, artifacts_dir, &progress_emitter) << "\n";
    return 0;
}

int RunJsonMode(const std::string& input_text) {
    json input_data;
    try {
        input_data = json::parse(input_text);
    } catch (const json::parse_error&) {
        EmitError("Invalid JSON input", "INVALID_JSON");
        std::cerr << "Invalid JSON input\n";
        return 1;
    }

    try {
        const std::string command = RequireString(input_data, "command");
        if (command != "extractCorpusMetadata") {
            throw std::runtime_error("Unknown command: " + command);
        }

        const std::string provided_corpus_name =
            input_data.contains("corpusName") && input_data["corpusName"].is_string()
                ? input_data["corpusName"].get<std::string>()
                : std::string();
        const std::string provided_artifacts_dir =
            input_data.contains("artifactsDir") && input_data["artifactsDir"].is_string()
                ? input_data["artifactsDir"].get<std::string>()
                : std::string();

        const std::string corpus_name = ResolveValue(provided_corpus_name, kInjectedCorpusName, "Corpus name");
        const std::string artifacts_dir = ResolveValue(
            provided_artifacts_dir,
            kInjectedArtifactsDir,
            "Artifacts directory");

        JsonProgressEmitter progress_emitter;
        progress_emitter.emit("Validating metadata request", 1);

        const std::string output = RunExtraction(corpus_name, artifacts_dir, &progress_emitter);
        EmitResult(json::parse(output));
        return 0;
    } catch (const std::exception& ex) {
        EmitError(ex.what(), "COMMAND_FAILED");
        std::cerr << ex.what() << "\n";
        return 1;
    }
}

} // namespace

int main(int argc, char** argv) {
    const std::string stdin_text = ReadStdin();

    try {
        if (HasNonWhitespace(stdin_text)) {
            return RunJsonMode(stdin_text);
        }
        return RunCliMode(argc, argv);
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
}
