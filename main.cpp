#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <fastlz.h>

#include "binops.hpp"

std::string read_full_file(const char* path)
{
    std::string out;
    std::ifstream stream(path, std::ios::binary);
    while (stream) {
        char buffer[1024];
        stream.read(buffer, sizeof(buffer));
        out.append(buffer, stream.gcount());
    }

    return out;
}

enum class setting_type {
    none = 0,
    bool_ = 1,
    number = 2,
    string = 3,
};

struct setting_value {
    std::string string_value;
    std::int32_t integer_value;
    double number_value;
};

void print_value(
        std::ostream& os,
        setting_type type,
        const setting_value& value)
{
    switch (type) {
        case setting_type::none:
            os << "nil";
            break;
        case setting_type::bool_:
            os << (value.integer_value ? "true" : "false");
            break;
        case setting_type::number:
            os << value.number_value;
            break;
        case setting_type::string:
            os << '"' << value.string_value << '"';
            break;
        default:
            os << "what?";
            break;
    }
}

struct settings_entry {
    std::string id;
    setting_type current_type;
    setting_type next_type;
    setting_value current_value;
    setting_value next_value;
};

struct read_value_result {
    setting_value value{};
    int read_size = 0;
};

read_value_result read_value(const char* it, setting_type type)
{
    if (type == setting_type::none)
        return {};

    if (type == setting_type::bool_) {
        auto bool_value = read_be<std::uint32_t>(it);
        return {
            .value{.integer_value{static_cast<std::int32_t>(bool_value)}},
            .read_size{4},
        };
    }

    if (type == setting_type::number) {
        auto bytes = read_be<std::uint64_t>(it);
        double number;
        std::memcpy(&number, &bytes, 8);
        return {
            .value{.number_value{number}},
            .read_size{8},
        };
    }

    if (type == setting_type::string) {
        auto size = read_be<std::uint32_t>(it);
        std::string string(size, '\0');
        std::memcpy(string.data(), it + 4, string.size());
        return {
            .value{.string_value{string}},
            .read_size{4 + (int)string.size()},
        };
    }

    return {};
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        const char* program_path = argv[0];
        if (program_path[0] == '\0')
            program_path = "noita-settings-dump";

        std::cerr << "Bad usage!\n";
        std::cerr << "Example:\n  ";
        std::cerr << program_path << " <path to mod_settings.bin>" << '\n';
        return 1;
    }

    std::string compressed = read_full_file(argv[1]);

    if (compressed.size() < 8) {
        std::cerr << "No compression file header.\n";
        return 2;
    }

    auto compressed_size = read_le<std::uint32_t>(compressed.data());
    auto decompressed_size = read_le<std::uint32_t>(compressed.data() + 4);

    if (compressed.size() - 8 != compressed_size) {
        std::cerr << "Bad compressed size.\n";
        std::cerr << "  Expected: " << compressed_size << '\n';
        std::cerr << "  Actual  : " << compressed.size() << '\n';
        return 2;
    }

    std::string output_buffer(decompressed_size, '\0');
    auto actual_size = fastlz_decompress(
            compressed.data() + 8,
            compressed_size,
            output_buffer.data(),
            output_buffer.size());

    if (actual_size == 0) {
        std::cerr << "Couldn't decompress file.\n";
        return 2;
    }

    if (actual_size != output_buffer.size()) {
        std::cerr << "Unexpected decompressed size.\n";
        std::cerr << "  Expected: " << output_buffer.size() << '\n';
        std::cerr << "  Actual  : " << actual_size << '\n';
        return 3;
    }

    const char* it = output_buffer.data();
    const char* end = output_buffer.data() + output_buffer.size();

    auto expected_entries = read_be<std::uint64_t>(it);
    it += 8;

    std::vector<settings_entry> entries;

    while (it < end) {
        std::string id(read_be<std::uint32_t>(it), '\0');
        it += 4;
        std::memcpy(id.data(), it, id.size());
        it += id.size();

        auto current_type{static_cast<setting_type>(read_be<std::uint32_t>(it))};
        it += 4;

        auto next_type{static_cast<setting_type>(read_be<std::uint32_t>(it))};
        it += 4;

        auto [current_value, csize] = read_value(it, current_type);
        it += csize;
        auto [next_value, nsize] = read_value(it, next_type);
        it += nsize;

        entries.push_back(settings_entry{
            .id = id,
            .current_type = current_type,
            .next_type = next_type,
            .current_value = current_value,
            .next_value = next_value,
        });
    }

    if (entries.size() != expected_entries) {
        std::cerr << "WARN: Actual and expected entries don't match\n";
        std::cerr << "  Expected: " << expected_entries << '\n';
        std::cerr << "  Actual  : " << entries.size() << '\n';
    }

    for (auto&& entry : entries) {
        std::cout << entry.id << '\n';
        std::cout << "  Current: ";
        print_value(std::cout, entry.current_type, entry.current_value);
        std::cout << '\n';

        std::cout << "  Next   : ";
        print_value(std::cout, entry.next_type, entry.next_value);
        std::cout << "\n\n";
    }
}
