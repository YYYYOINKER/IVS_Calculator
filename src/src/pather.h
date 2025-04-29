#pragma once
#include <string>
/**
 * @file pather.h
 * @brief Utility for resolving relative asset paths.
 *
 * Adds a base asset directory prefix to a given filename.
 */
#ifndef ASSET_PATH
#define ASSET_PATH "./"
#endif

/**
 * @brief Prepends the asset path to a given filename.
 * @param path Relative path to asset.
 * @return Full path with asset directory prepended.
 */
inline std::string pather(const std::string& path) {
    return std::string(ASSET_PATH) + path;
}


