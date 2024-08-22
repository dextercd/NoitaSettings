#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "binops.hpp"
#include "utils.hpp"

struct PhysicsObject {
    std::uint64_t a;
    std::uint32_t b;
    float x;
    float y;
    float rot_radians;
    double f;
    double g;
    double h;
    double i;
    double j;
    bool k;
    bool l;
    bool m;
    bool n;
    bool o;
    float z;
    std::uint32_t width;
    std::uint32_t height;
    std::vector<std::uint32_t> colors;
};

struct RenderObject {
    sf::Sprite sprite;
    std::unique_ptr<sf::Texture> texture;
    const PhysicsObject* physics_object;
};

const float degrees_in_radians = 57.2957795131;

struct read_vec_uint32_result {
    const char* ptr;
    std::vector<std::uint32_t> data;
};

read_vec_uint32_result read_vec_uint32(const char* ptr)
{
    auto count = read_be<std::uint32_t>(ptr);
    std::vector<std::uint32_t> result(count);

    ptr += 4;
    for (auto& out : result) {
        out = read_be<std::uint32_t>(ptr);
        ptr += 4;
    }

    return {ptr, result};
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        const char* program_path = argv[0];
        if (program_path[0] == '\0')
            program_path = "noita-png-petri";

        std::cerr << "Bad usage!\n";
        std::cerr << "Example:\n  ";
        std::cerr << program_path << " <path to .png_petri file>" << '\n';
        return 1;
    }

    auto file_contents = read_compressed_file(argv[1]);
    auto data = file_contents.c_str();
    auto data_end = data + file_contents.size();

    auto version = read_be<std::uint32_t>(data);
    auto width_q = read_be<std::uint32_t>(data + 4);
    auto height_q = read_be<std::uint32_t>(data + 8);

    if (version != 24 || width_q != 512 || height_q != 512) {
        std::cerr << "Unexpected header:\n";
        std::cerr << "  version: " << version << '\n';
        std::cerr << "  width?: " << width_q << '\n';
        std::cerr << "  height?: " << height_q << '\n';
        return 1;
    }

    auto world_cells_start = data + 12;

    std::vector<std::uint8_t> hp_values_q(512 * 512);
    std::memcpy(hp_values_q.data(), world_cells_start, 512 * 512);

    auto material_names_start = world_cells_start + 512 * 512;
    auto material_name_count = read_be<std::uint32_t>(material_names_start);
    std::vector<std::string> material_names(material_name_count);

    auto material_names_ptr = material_names_start + 4;
    for (int i = 0; i != material_name_count; ++i) {
        auto size = read_be<std::uint32_t>(material_names_ptr);
        material_names[i].resize(size);
        std::memcpy(material_names[i].data(), material_names_ptr + 4, size);
        material_names_ptr += 4 + size;
    }

    std::cout << "Materials:\n";
    for (const auto& material_name : material_names) {
        std::cout << "  - " << material_name << '\n';
    }

    auto [physics_objects_start, custom_world_colors] =
            read_vec_uint32(material_names_ptr);

    auto physics_object_count = read_be<std::uint32_t>(physics_objects_start);
    auto current_object = physics_objects_start + 4;

    // assert version == 24
    std::vector<PhysicsObject> physics_objects(physics_object_count);

    for (auto i = 0; i != physics_object_count; ++i) {
        auto& into = physics_objects[i];

        into.a = read_be<std::uint64_t>(current_object);
        into.b = read_be<std::uint32_t>(current_object + 8);
        into.x = read_be<float>(current_object + 12);
        into.y = read_be<float>(current_object + 16);
        into.rot_radians = read_be<float>(current_object + 20);
        into.f = read_be<double>(current_object + 24);
        into.g = read_be<double>(current_object + 32);
        into.h = read_be<double>(current_object + 40);
        into.i = read_be<double>(current_object + 48);
        into.j = read_be<double>(current_object + 56);
        into.k = read_be<bool>(current_object + 64);
        into.l = read_be<bool>(current_object + 65);
        into.m = read_be<bool>(current_object + 66);
        into.n = read_be<bool>(current_object + 67);
        into.o = read_be<bool>(current_object + 68);
        into.z = read_be<float>(current_object + 69);
        into.width = read_be<std::uint32_t>(current_object + 73);
        into.height = read_be<std::uint32_t>(current_object + 77);

        auto image_size = into.width * into.height;
        into.colors.resize(image_size);

        auto image_data = current_object + 81;
        for (int i = 0; i != image_size; ++i) {
            into.colors[i] = read_be<std::uint32_t>(image_data);
            image_data += 4;
        }

        std::cout << "into.a: " << into.a << '\n';
        std::cout << "into.b: " << into.b << '\n';
        std::cout << "into.x: " << into.x << '\n';
        std::cout << "into.y: " << into.y << '\n';
        std::cout << "into.rot_radians: " << into.rot_radians << '\n';
        std::cout << "into.f: " << into.f << '\n';
        std::cout << "into.g: " << into.g << '\n';
        std::cout << "into.h: " << into.h << '\n';
        std::cout << "into.i: " << into.i << '\n';
        std::cout << "into.j: " << into.j << '\n';
        std::cout << "into.k: " << into.k << '\n';
        std::cout << "into.l: " << into.l << '\n';
        std::cout << "into.m: " << into.m << '\n';
        std::cout << "into.n: " << into.n << '\n';
        std::cout << "into.o: " << into.o << '\n';
        std::cout << "into.z: " << into.z << '\n';
        std::cout << "into.width: " << into.width << '\n';
        std::cout << "into.height: " << into.height << '\n';
        std::cout << "\n\n\n";

        current_object = image_data;
    }

    auto unknown2_start = current_object;
    auto unknown2_count = read_be<std::uint32_t>(unknown2_start);

    std::cout << "Unhandled offset: " << current_object - data << '\n';
    std::cout << "Unhandled bytes: " << data_end - current_object << '\n';

    //
    // Display
    //

    sf::Vector2f size(950.f, 800.f);
    sf::RenderWindow window(
        sf::VideoMode(size.x, size.y),
        "World Viewer"
    );

    window.setVerticalSyncEnabled(true);

    float zoom = 1;
    float x = 0, y = 0;

    auto set_view = [&] () {
        sf::View view(sf::Vector2f(x, y), size / zoom);
        window.setView(view);
    };

    sf::Texture world_texture;
    world_texture.create(0x200, 0x200);
    auto custom_color_it = custom_world_colors.begin();
    for (int i = 0; i != 512 * 512; ++i) {
        auto posx = i % 512;
        auto posy = i / 512;
        auto material = hp_values_q[i] & (~0x80);
        auto custom_color = (hp_values_q[i] & 0x80) != 0;
        if (custom_color) {
            std::uint32_t color = *custom_color_it;
            world_texture.update((unsigned char*)&color, 1, 1, posx, posy);
            ++custom_color_it;
        }
    }

    sf::Sprite world_sprite;
    world_sprite.setTexture(world_texture);

    std::vector<RenderObject> render_objects;
    for (const auto& physics_object : physics_objects) {
        auto& ro = render_objects.emplace_back();

        ro.physics_object = &physics_object;

        ro.texture = std::make_unique<sf::Texture>();
        ro.texture->create(physics_object.width, physics_object.height);
        ro.texture->update((unsigned char*)physics_object.colors.data());

        ro.sprite.setTexture(*ro.texture);
        ro.sprite.setPosition({physics_object.x - 512, physics_object.y});
        ro.sprite.setRotation(physics_object.rot_radians * degrees_in_radians);
    }

    std::sort(std::begin(render_objects), std::end(render_objects),
        [] (auto& a, auto& b) {
            return a.physics_object->z > b.physics_object->z;
        });

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::Resized) {
                size = sf::Vector2f(event.size.width, event.size.height);
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Add)
        ||  sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Equal)) {
            zoom += 0.02;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Subtract)) {
            zoom -= 0.02;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))  x -= 1.f / zoom;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) x += 1.f / zoom;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))    y -= 1.f / zoom;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))  y += 1.f / zoom;

        window.clear(sf::Color::Black);

        window.draw(world_sprite);
        for (auto&& render_object : render_objects)
            window.draw(render_object.sprite);

        set_view();
        window.display();
    }
}
