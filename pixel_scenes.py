from dataclasses import dataclass
import sys
import struct
import typing


@dataclass
class PixelScene:
    x: int
    y: int
    material_filename: str
    colors_filename: str
    background_filename: str
    skip_biome_checks: bool
    skip_edge_textures: bool
    background_z_index: int
    just_load_an_entity: str
    clean_area_before: bool
    DEBUG_RELOAD_ME: bool
    color_material: dict[int, int]  # Color -> CellData.type


@dataclass
class Image:
    filename: str
    x: int
    y: int


@dataclass
class PixelSceneFile:
    version: int
    magic_num: int

    pending_list: list[PixelScene]
    placed_list: list[PixelScene]
    background_images: list[Image]  # Always empty?


def read_string(r: typing.BinaryIO):
    (length,) = struct.unpack(">i", r.read(4))
    return r.read(length).decode("utf-8")


def read_pixel_scene_file(read_stream: typing.BinaryIO) -> PixelSceneFile:
    version, magic_num = struct.unpack(">ii", read_stream.read(8))

    if magic_num != 0x2F0AA9F:
        raise Exception("Unsupported old pixel scenes binary file version")

    if version != 3:
        raise Exception("Unsupported version of pixel scenes binary file")

    pending_list = read_pixel_scene_list(read_stream)
    placed_list = read_pixel_scene_list(read_stream)
    background_images = read_image_list(read_stream)

    return PixelSceneFile(
        version=version,
        magic_num=magic_num,
        pending_list=pending_list,
        placed_list=placed_list,
        background_images=background_images,
    )


def read_pixel_scene_list(read_stream: typing.BinaryIO) -> list[PixelScene]:
    (count,) = struct.unpack(">i", read_stream.read(4))
    return [read_pixel_scene(read_stream) for _ in range(count)]


def read_pixel_scene(read_stream: typing.BinaryIO) -> PixelScene:
    x, y = struct.unpack(">ii", read.read(8))
    material_filename = read_string(read)
    colors_filename = read_string(read)
    background_filename = read_string(read)
    skip_biome_checks, skip_edge_textures = struct.unpack("??", read.read(2))
    (background_z_index,) = struct.unpack(">i", read.read(4))
    just_load_an_entity = read_string(read)
    clean_area_before, DEBUG_RELOAD_ME = struct.unpack("??", read.read(2))
    (color_material_count,) = struct.unpack(">I", read.read(4))

    color_material = {}
    for _ in range(color_material_count):
        (color,) = struct.unpack("<I", read.read(4))
        color = color >> 8
        (cell_type,) = struct.unpack(">i", read.read(4))
        color_material[color] = cell_type

    return PixelScene(
        x=x,
        y=y,
        material_filename=material_filename,
        colors_filename=colors_filename,
        background_filename=background_filename,
        skip_biome_checks=skip_biome_checks,
        skip_edge_textures=skip_edge_textures,
        background_z_index=background_z_index,
        just_load_an_entity=just_load_an_entity,
        clean_area_before=clean_area_before,
        DEBUG_RELOAD_ME=DEBUG_RELOAD_ME,
        color_material=color_material,
    )


def read_image_list(read_stream: typing.BinaryIO) -> list[Image]:
    (count,) = struct.unpack(">i", read_stream.read(4))
    return [read_image(read_stream) for _ in range(count)]


def read_image(read_stream: typing.BinaryIO) -> Image:
    x, y = struct.unpack(">ii", read.read(8))
    filename = read_string(read)
    return Image(
        x=x,
        y=y,
        filename=filename,
    )


if __name__ == "__main__":
    read = sys.stdin.buffer
    print(read_pixel_scene_file(read))
