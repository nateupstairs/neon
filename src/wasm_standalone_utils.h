#pragma once

#include <emscripten/emscripten.h>
#include <cstdlib>
#include <cstring>
#include <string>

extern "C" {

EMSCRIPTEN_KEEPALIVE
void* wasm_malloc (size_t size) {
    return malloc (size);
}

EMSCRIPTEN_KEEPALIVE
void wasm_free (void* ptr) {
    free (ptr);
}

}

inline char* wasm_create_string (const std::string& str) {
    char* buf = (char*) malloc (str.size() + 1);
    memcpy (buf, str.c_str(), str.size() + 1);
    return buf;
}
