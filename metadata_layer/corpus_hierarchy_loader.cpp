#include "corpus_hierarchy_loader.hpp"

#include <map>
#include <stdexcept>

#include <sqlite3.h>

namespace teknegram {

namespace {

class SqliteDb {
    public:
        explicit SqliteDb(const std::string& path)
            : db_(0) {
            if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
                const std::string err = db_ ? sqlite3_errmsg(db_) : "unknown sqlite open error";
                if (db_) {
                    sqlite3_close(db_);
                }
                throw std::runtime_error("Failed to open metadata DB: " + err);
            }
        }

        ~SqliteDb() {
            if (db_) {
                sqlite3_close(db_);
            }
        }

        sqlite3* get() const {
            return db_;
        }

    private:
        sqlite3* db_;
};

std::map<std::uint32_t, std::vector<std::string> > LoadDocumentPaths(sqlite3* db) {
    const char* sql =
        "SELECT d.document_id, ds.depth, fs.segment_text "
        "FROM document d "
        "LEFT JOIN document_segment ds ON ds.document_id = d.document_id "
        "LEFT JOIN folder_segment fs ON fs.segment_id = ds.segment_id "
        "ORDER BY d.document_id ASC, ds.depth ASC;";

    sqlite3_stmt* stmt = 0;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare hierarchy query: " + std::string(sqlite3_errmsg(db)));
    }

    std::map<std::uint32_t, std::vector<std::string> > paths;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const std::uint32_t document_id = static_cast<std::uint32_t>(sqlite3_column_int(stmt, 0));
        if (sqlite3_column_type(stmt, 1) == SQLITE_NULL || sqlite3_column_type(stmt, 2) == SQLITE_NULL) {
            if (paths.find(document_id) == paths.end()) {
                paths[document_id] = std::vector<std::string>();
            }
            continue;
        }

        const unsigned char* segment_text = sqlite3_column_text(stmt, 2);
        if (!segment_text) {
            continue;
        }
        paths[document_id].push_back(reinterpret_cast<const char*>(segment_text));
    }

    const int rc = sqlite3_finalize(stmt);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to finalize hierarchy query: " + std::string(sqlite3_errmsg(db)));
    }
    return paths;
}

std::size_t GetOrCreateChild(CorpusHierarchy* hierarchy,
                             std::size_t parent_index,
                             const std::string& child_name,
                             std::map<std::pair<std::size_t, std::string>, std::size_t>* child_lookup) {
    const std::pair<std::size_t, std::string> key(parent_index, child_name);
    std::map<std::pair<std::size_t, std::string>, std::size_t>::const_iterator it = child_lookup->find(key);
    if (it != child_lookup->end()) {
        return it->second;
    }

    const std::size_t child_index = hierarchy->nodes.size();
    hierarchy->nodes.push_back(HierarchyNode(child_name));
    hierarchy->nodes[parent_index].child_indices.push_back(child_index);
    (*child_lookup)[key] = child_index;
    return child_index;
}

} // namespace

CorpusHierarchy CorpusHierarchyLoader::load(const std::string& db_path,
                                            const std::string& corpus_name) const {
    SqliteDb db(db_path);
    const std::map<std::uint32_t, std::vector<std::string> > doc_paths = LoadDocumentPaths(db.get());

    CorpusHierarchy hierarchy(corpus_name);
    std::map<std::pair<std::size_t, std::string>, std::size_t> child_lookup;

    for (std::map<std::uint32_t, std::vector<std::string> >::const_iterator it = doc_paths.begin();
         it != doc_paths.end();
         ++it) {
        std::size_t node_index = hierarchy.root_index;
        for (std::size_t i = 0; i < it->second.size(); ++i) {
            node_index = GetOrCreateChild(&hierarchy, node_index, it->second[i], &child_lookup);
        }
        hierarchy.nodes[node_index].direct_document_ids.push_back(it->first);
    }

    return hierarchy;
}

} // namespace teknegram
