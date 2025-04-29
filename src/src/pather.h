#pragma once
#include <string>

#ifndef ASSET_PATH
#define ASSET_PATH "./"
#endif

inline std::string pather(const std::string& path) {
    return std::string(ASSET_PATH) + path;
}

