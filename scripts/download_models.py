#!/usr/bin/env python3
"""
download_models.py — Downloads CLIP ONNX models from HuggingFace Hub.

This script pulls the pre-exported OpenAI CLIP ViT-B/32 model files
needed for bobfilez's Vector-Semantic Search engine.

Files downloaded:
  1. clip-image-vit-b32.onnx  (~350MB)  — Vision encoder
  2. clip-text-vit-b32.onnx   (~250MB)  — Text encoder
  3. bpe_simple_vocab_16e6.txt (~1.4MB)  — BPE tokenizer vocabulary

Usage:
  python scripts/download_models.py [--output-dir=.models]

The engine expects these files in the .models/ directory at project root.
"""

import os
import sys
import urllib.request
import hashlib
import argparse

# Model URLs (HuggingFace ONNX exports of OpenAI CLIP ViT-B/32)
MODELS = {
    "clip-image-vit-b32.onnx": {
        "url": "https://huggingface.co/openai/clip-vit-base-patch32/resolve/main/onnx/model.onnx",
        "description": "CLIP Vision Encoder (ViT-B/32) — processes 224x224 RGB images into 512-dim vectors",
        "size_mb": 350,
    },
    "clip-text-vit-b32.onnx": {
        "url": "https://huggingface.co/openai/clip-vit-base-patch32/resolve/main/onnx/model.onnx_data",
        "description": "CLIP Text Encoder (ViT-B/32) — processes BPE token sequences into 512-dim vectors",
        "size_mb": 250,
    },
    "bpe_simple_vocab_16e6.txt": {
        "url": "https://raw.githubusercontent.com/openai/CLIP/main/clip/bpe_simple_vocab_16e6.txt.gz",
        "description": "BPE vocabulary (49152 tokens) for CLIP text tokenization",
        "size_mb": 1.4,
    },
}


def download_file(url: str, dest: str, description: str):
    """Download a file with progress indication."""
    if os.path.exists(dest):
        size = os.path.getsize(dest)
        if size > 1000:
            print(f"  [SKIP] {os.path.basename(dest)} already exists ({size:,} bytes)")
            return True

    print(f"  [DOWNLOADING] {description}")
    print(f"    URL: {url}")
    print(f"    Destination: {dest}")

    try:
        def report_progress(block_num, block_size, total_size):
            downloaded = block_num * block_size
            if total_size > 0:
                pct = min(100, downloaded * 100 // total_size)
                bar_len = 40
                filled = bar_len * pct // 100
                bar = "█" * filled + "░" * (bar_len - filled)
                print(f"\r    [{bar}] {pct}% ({downloaded // 1024 // 1024}MB / {total_size // 1024 // 1024}MB)", end="", flush=True)
            else:
                print(f"\r    Downloaded: {downloaded // 1024 // 1024}MB", end="", flush=True)

        urllib.request.urlretrieve(url, dest, reporthook=report_progress)
        print()  # Newline after progress bar
        print(f"  [OK] {os.path.basename(dest)} ({os.path.getsize(dest):,} bytes)")
        return True

    except Exception as e:
        print(f"\n  [ERROR] Failed to download: {e}")
        print(f"    You can manually download from: {url}")
        return False


def main():
    parser = argparse.ArgumentParser(description="Download CLIP ONNX models for bobfilez semantic search")
    parser.add_argument("--output-dir", default=".models", help="Directory to save models (default: .models)")
    args = parser.parse_args()

    output_dir = args.output_dir
    os.makedirs(output_dir, exist_ok=True)

    print("=" * 70)
    print("  bobfilez Model Downloader")
    print("  Downloads CLIP ViT-B/32 ONNX models for Vector-Semantic Search")
    print("=" * 70)
    print()

    total = len(MODELS)
    success = 0

    for i, (filename, info) in enumerate(MODELS.items(), 1):
        print(f"[{i}/{total}] {filename} (~{info['size_mb']}MB)")
        dest = os.path.join(output_dir, filename)
        if download_file(info["url"], dest, info["description"]):
            success += 1
        print()

    # Decompress the vocab if it's gzipped
    vocab_gz = os.path.join(output_dir, "bpe_simple_vocab_16e6.txt")
    if vocab_gz.endswith(".txt") and os.path.exists(vocab_gz):
        # Check if it's actually gzipped content
        try:
            with open(vocab_gz, "rb") as f:
                magic = f.read(2)
            if magic == b'\x1f\x8b':  # gzip magic number
                import gzip
                print("[DECOMPRESS] Extracting BPE vocabulary...")
                with gzip.open(vocab_gz, "rb") as f_in:
                    data = f_in.read()
                with open(vocab_gz, "wb") as f_out:
                    f_out.write(data)
                print(f"  [OK] Decompressed to {len(data):,} bytes")
        except Exception as e:
            print(f"  [WARN] Could not decompress: {e}")

    print("=" * 70)
    print(f"  Download complete: {success}/{total} files")
    print(f"  Models saved to: {os.path.abspath(output_dir)}")
    print()
    print("  To use with bobfilez:")
    print(f"    Set FO_CLIP_VISION_MODEL={os.path.join(output_dir, 'clip-image-vit-b32.onnx')}")
    print(f"    Set FO_CLIP_TEXT_MODEL={os.path.join(output_dir, 'clip-text-vit-b32.onnx')}")
    print(f"    Set FO_CLIP_VOCAB={os.path.join(output_dir, 'bpe_simple_vocab_16e6.txt')}")
    print("=" * 70)

    return 0 if success == total else 1


if __name__ == "__main__":
    sys.exit(main())
