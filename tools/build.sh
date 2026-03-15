#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

g++ -std=c++11 -Wall -Wextra -pedantic \
  main.cpp \
  shared_layer/progress_emitter.cpp \
  metadata_layer/corpus_hierarchy_loader.cpp \
  counting_layer/document_counter.cpp \
  aggregation_layer/subcorpus_aggregator.cpp \
  output_layer/json_serializer.cpp \
  orchestration_layer/corpus_metadata_engine.cpp \
  -lsqlite3 \
  -o corpus_metadata_pipeline

echo "Build passed."
