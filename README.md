# Corpus Metadata Pipeline (C++11)

This project reads existing corpus artefacts and extracts hierarchical
JSON metadata for the corpus and its folder-based subcorpora.

## Layout

- `metadata_layer/`
- `counting_layer/`
- `aggregation_layer/`
- `output_layer/`
- `shared_layer/`
- `orchestration_layer/`

## Requirements

- Existing corpus artefacts built by `corpus_builder/`
- SQLite3 development library

Required input files in the artefacts directory:

- `corpus.db`
- `doc_ranges.bin`
- `word.bin`
- `lemma.bin`
- `pos.bin`

## What It Produces

The pipeline emits nested JSON describing the corpus hierarchy and
counts at every node:

- `docs`
- `words`
- `lemmas`
- `types`

Folder-based subcorpora are reconstructed from `corpus.db`, so this
works even when no semantic TSV rules were supplied during corpus build.

## Build

```bash
./tools/build.sh
```

This builds `corpus_metadata_pipeline`.

## Run

CLI mode:

```bash
./corpus_metadata_pipeline \
  "BNC" \
  /path/to/corpus_output
```

The first argument is the corpus name and the second is the artefacts
directory.

If your embedding layer injects those values into `main.cpp`, the binary
can also run without CLI arguments.

JSON mode:

```json
{
  "command": "extractCorpusMetadata",
  "corpusName": "BNC",
  "artifactsDir": "/path/to/corpus_output"
}
```

If `corpusName` or `artifactsDir` are omitted from JSON input, the
program falls back to injected values configured in `main.cpp`.

## Output Shape

Example:

```json
{
  "corpus_name": "BNC",
  "docs": 25,
  "words": 5000,
  "lemmas": 3200,
  "types": 900,
  "subcorpora": [
    {
      "name": "Speaking",
      "docs": 13,
      "words": 1000,
      "lemmas": 850,
      "types": 100,
      "subcorpora": [
        {
          "name": "Political speeches",
          "docs": 3,
          "words": 4000,
          "lemmas": 3050,
          "types": 700,
          "subcorpora": []
        }
      ]
    }
  ]
}
```

If the corpus has no nested folder structure, the root object is still
reported and `subcorpora` is empty.
