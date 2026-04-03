# Vector-Semantic Search Design Document

## 1. Overview
The bobfilez file organization engine relies extensively on exact-match or near-match pattern searching (regex, globs, fuzzy paths). However, to search a vast library of images, standard filenames are insufficient. 

**Vector-Semantic Search** utilizes an OpenAI CLIP (Contrastive Language-Image Pretraining) model to map both text strings and images into the same multi-dimensional mathematical space (embedding vector). This allows a user to type a natural language query like `"A dog sitting on the grass"` and immediately find images depicting exactly that, regardless of what the file is named (`IMG_9821.JPG`).

## 2. Technical Stack
*   **ONNX Runtime C++ API**: Used to execute the machine learning models locally.
*   **Vision Model (`clip-image.onnx`)**: Takes a 224x224 RGB image and outputs a 512-dimensional float vector.
*   **Text Model (`clip-text.onnx`)**: Takes a tokenized text string and outputs a 512-dimensional float vector.
*   **SQLite3**: Persists the computed image embeddings to disk.
*   **Cosine Similarity**: Measures the angle between the Text embedding vector and each Image embedding vector to compute relevance.

## 3. Workflow

### A. Indexing (Background Process)
1.  User adds a directory to the `bobfilez` library.
2.  Background thread identifies all un-indexed images.
3.  Image is resized to 224x224, centered, normalized with ImageNet means/stds.
4.  ONNX Runtime infers the `clip-image` embedding.
5.  512-d float array is serialized as a BLOB into the `file_embeddings` SQLite table.

### B. Searching
1.  User types `"sunsets over the ocean"` in the `SearchPanel.qml`.
2.  BPE (Byte Pair Encoding) Tokenizer converts the string into an integer array sequence.
3.  ONNX Runtime infers the `clip-text` embedding.
4.  C++ engine performs an optimized dot-product (cosine similarity since vectors are L2-normalized) against all stored BLOBs in SQLite.
5.  Results are sorted by similarity score (descending) and returned to the UI in milliseconds.

## 4. Submodules & Dependencies
No new submodules are strictly required if we implement a basic BPE tokenizer in C++ (or port a lightweight version) and utilize the existing `onnxruntime` dependency. OpenCV handles the image scaling and preprocessing.

## 5. Architecture
*   `IClipSearchEngine`: Main interface for generating embeddings and searching.
*   `ClipOnnxBackend`: The underlying ONNX wrapper handling memory management and inference.
*   `Tokenizer`: C++ BPE vocabulary dictionary loader for CLIP tokens.
*   `DatabaseManager`: Extends existing schema with `CREATE TABLE file_embeddings (file_id INTEGER PRIMARY KEY, vector BLOB)`.
