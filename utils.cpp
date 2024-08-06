#include <cstdint>
#include <fstream>
#include <stdexcept>

#include <fastlz.h>

#include "binops.hpp"

#include "utils.hpp"

std::string read_file(const char* path)
{
    std::ifstream stream(path, std::ios::binary);
    return read_stream(stream);
}

std::string read_stream(std::istream& stream) {
    std::string out;
    while (stream) {
        char buffer[1024];
        stream.read(buffer, sizeof(buffer));
        out.append(buffer, stream.gcount());
    }

    return out;
}

std::string read_compressed_file(const char* path)
{
    std::string compressed = read_file(path);
    return decompress_data(compressed);
}

std::string decompress_data(const std::string& compressed)
{
    if (compressed.size() < 8)
        throw std::runtime_error{"No compression file header"};

    auto compressed_size = read_le<std::uint32_t>(compressed.data());
    auto decompressed_size = read_le<std::uint32_t>(compressed.data() + 4);

    if (compressed.size() - 8 != compressed_size)
        throw std::runtime_error{"Bad compressed size"};

    if (compressed_size == decompressed_size) {
        return compressed.substr(8);
    }

    std::string output_buffer(decompressed_size, '\0');
    auto actual_size = fastlz_decompress(
            compressed.data() + 8,
            compressed_size,
            output_buffer.data(),
            output_buffer.size());

    if (actual_size == 0)
        throw std::runtime_error{"Couldn't decompress file.\n"};

    if (actual_size != output_buffer.size())
        throw std::runtime_error{"Unexpected decompressed size"};

    return output_buffer;
}

std::string compress_data(const std::string& data)
{
    std::string result(8 + data.size() * 2, '\0');
    auto new_size = fastlz_compress_level(2, data.data(), data.size(), result.data() + 8);
    if (new_size <= 0) {
        throw std::runtime_error{"Error when compressing"};
    }

    if (new_size >= data.size()) {
        std::copy(data.begin(), data.end(), result.begin() + 8);
        new_size = data.size();
    }

    write_le(result.data(), (std::int32_t)new_size);
    write_le(result.data() + 4, (std::int32_t)data.size());

    result.resize(new_size + 8);
    return result;
}
