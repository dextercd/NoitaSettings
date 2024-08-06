#ifndef NOITA_SETTINGS_UTILS_HPP
#define NOITA_SETTINGS_UTILS_HPP

#include <string>

std::string read_file(const char* path);
std::string read_stream(std::istream& stream);
std::string read_compressed_file(const char* path);
std::string decompress_data(const std::string& compressed);

std::string compress_data(const std::string& data);

#endif // Header guard
