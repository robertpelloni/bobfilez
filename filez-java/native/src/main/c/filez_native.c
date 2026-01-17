/**
 * filez_native.c - JNI implementations for BLAKE3 and XXHash64
 * 
 * Build requirements:
 * - BLAKE3 C library (https://github.com/BLAKE3-team/BLAKE3)
 * - XXHash library (https://github.com/Cyan4973/xxHash)
 * - JDK with jni.h
 * 
 * Compile (Windows MSVC):
 *   cl /LD /I"%JAVA_HOME%\include" /I"%JAVA_HOME%\include\win32" ^
 *      /I<blake3_dir> /I<xxhash_dir> ^
 *      filez_native.c blake3.c xxhash.c /Fe:filez_native.dll
 * 
 * Compile (Linux/macOS GCC):
 *   gcc -shared -fPIC -o libfilez_native.so \
 *       -I"$JAVA_HOME/include" -I"$JAVA_HOME/include/linux" \
 *       -I<blake3_dir> -I<xxhash_dir> \
 *       filez_native.c blake3.c xxhash.c
 */

#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Include BLAKE3 and XXHash headers */
#include "blake3.h"
#define XXH_INLINE_ALL
#include "xxhash.h"

#include "com_filez_jni_Blake3Jni.h"
#include "com_filez_jni_XxHash64Jni.h"

/* Buffer size for streaming hash */
#define DEFAULT_BUFFER_SIZE (64 * 1024)

/* Convert bytes to hex string */
static char* bytes_to_hex(const uint8_t* bytes, size_t len) {
    char* hex = (char*)malloc(len * 2 + 1);
    if (!hex) return NULL;
    
    static const char hex_chars[] = "0123456789abcdef";
    for (size_t i = 0; i < len; i++) {
        hex[i * 2] = hex_chars[(bytes[i] >> 4) & 0xF];
        hex[i * 2 + 1] = hex_chars[bytes[i] & 0xF];
    }
    hex[len * 2] = '\0';
    return hex;
}

/* ============== BLAKE3 JNI Implementation ============== */

JNIEXPORT jstring JNICALL Java_com_filez_jni_Blake3Jni_nativeHashFile
  (JNIEnv *env, jclass cls, jstring jpath) {
    
    const char* path = (*env)->GetStringUTFChars(env, jpath, NULL);
    if (!path) return NULL;
    
    FILE* file = fopen(path, "rb");
    (*env)->ReleaseStringUTFChars(env, jpath, path);
    
    if (!file) {
        return NULL;
    }
    
    blake3_hasher hasher;
    blake3_hasher_init(&hasher);
    
    uint8_t buffer[DEFAULT_BUFFER_SIZE];
    size_t bytes_read;
    
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        blake3_hasher_update(&hasher, buffer, bytes_read);
    }
    
    fclose(file);
    
    uint8_t output[BLAKE3_OUT_LEN];
    blake3_hasher_finalize(&hasher, output, BLAKE3_OUT_LEN);
    
    char* hex = bytes_to_hex(output, BLAKE3_OUT_LEN);
    if (!hex) return NULL;
    
    jstring result = (*env)->NewStringUTF(env, hex);
    free(hex);
    
    return result;
}

JNIEXPORT jstring JNICALL Java_com_filez_jni_Blake3Jni_nativeHashBytes
  (JNIEnv *env, jclass cls, jbyteArray jdata) {
    
    jsize len = (*env)->GetArrayLength(env, jdata);
    jbyte* data = (*env)->GetByteArrayElements(env, jdata, NULL);
    if (!data) return NULL;
    
    uint8_t output[BLAKE3_OUT_LEN];
    blake3_hasher hasher;
    blake3_hasher_init(&hasher);
    blake3_hasher_update(&hasher, (const uint8_t*)data, len);
    blake3_hasher_finalize(&hasher, output, BLAKE3_OUT_LEN);
    
    (*env)->ReleaseByteArrayElements(env, jdata, data, JNI_ABORT);
    
    char* hex = bytes_to_hex(output, BLAKE3_OUT_LEN);
    if (!hex) return NULL;
    
    jstring result = (*env)->NewStringUTF(env, hex);
    free(hex);
    
    return result;
}

JNIEXPORT jstring JNICALL Java_com_filez_jni_Blake3Jni_nativeHashFileStreaming
  (JNIEnv *env, jclass cls, jstring jpath, jint bufferSize) {
    
    const char* path = (*env)->GetStringUTFChars(env, jpath, NULL);
    if (!path) return NULL;
    
    FILE* file = fopen(path, "rb");
    (*env)->ReleaseStringUTFChars(env, jpath, path);
    
    if (!file) {
        return NULL;
    }
    
    size_t buf_size = (bufferSize > 0) ? (size_t)bufferSize : DEFAULT_BUFFER_SIZE;
    uint8_t* buffer = (uint8_t*)malloc(buf_size);
    if (!buffer) {
        fclose(file);
        return NULL;
    }
    
    blake3_hasher hasher;
    blake3_hasher_init(&hasher);
    
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, buf_size, file)) > 0) {
        blake3_hasher_update(&hasher, buffer, bytes_read);
    }
    
    free(buffer);
    fclose(file);
    
    uint8_t output[BLAKE3_OUT_LEN];
    blake3_hasher_finalize(&hasher, output, BLAKE3_OUT_LEN);
    
    char* hex = bytes_to_hex(output, BLAKE3_OUT_LEN);
    if (!hex) return NULL;
    
    jstring result = (*env)->NewStringUTF(env, hex);
    free(hex);
    
    return result;
}

/* ============== XXHash64 JNI Implementation ============== */

JNIEXPORT jlong JNICALL Java_com_filez_jni_XxHash64Jni_nativeHashFile
  (JNIEnv *env, jclass cls, jstring jpath) {
    
    const char* path = (*env)->GetStringUTFChars(env, jpath, NULL);
    if (!path) return 0;
    
    FILE* file = fopen(path, "rb");
    (*env)->ReleaseStringUTFChars(env, jpath, path);
    
    if (!file) {
        return 0;
    }
    
    XXH64_state_t* state = XXH64_createState();
    if (!state) {
        fclose(file);
        return 0;
    }
    
    XXH64_reset(state, 0);
    
    uint8_t buffer[DEFAULT_BUFFER_SIZE];
    size_t bytes_read;
    
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        XXH64_update(state, buffer, bytes_read);
    }
    
    fclose(file);
    
    XXH64_hash_t hash = XXH64_digest(state);
    XXH64_freeState(state);
    
    return (jlong)hash;
}

JNIEXPORT jlong JNICALL Java_com_filez_jni_XxHash64Jni_nativeHashBytes
  (JNIEnv *env, jclass cls, jbyteArray jdata) {
    
    jsize len = (*env)->GetArrayLength(env, jdata);
    jbyte* data = (*env)->GetByteArrayElements(env, jdata, NULL);
    if (!data) return 0;
    
    XXH64_hash_t hash = XXH64(data, len, 0);
    
    (*env)->ReleaseByteArrayElements(env, jdata, data, JNI_ABORT);
    
    return (jlong)hash;
}

JNIEXPORT jlong JNICALL Java_com_filez_jni_XxHash64Jni_nativeHashFileWithSeed
  (JNIEnv *env, jclass cls, jstring jpath, jlong seed) {
    
    const char* path = (*env)->GetStringUTFChars(env, jpath, NULL);
    if (!path) return 0;
    
    FILE* file = fopen(path, "rb");
    (*env)->ReleaseStringUTFChars(env, jpath, path);
    
    if (!file) {
        return 0;
    }
    
    XXH64_state_t* state = XXH64_createState();
    if (!state) {
        fclose(file);
        return 0;
    }
    
    XXH64_reset(state, (XXH64_hash_t)seed);
    
    uint8_t buffer[DEFAULT_BUFFER_SIZE];
    size_t bytes_read;
    
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        XXH64_update(state, buffer, bytes_read);
    }
    
    fclose(file);
    
    XXH64_hash_t hash = XXH64_digest(state);
    XXH64_freeState(state);
    
    return (jlong)hash;
}
