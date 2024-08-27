from dataclasses import dataclass
import sys
import struct
import typing


@dataclass
class Background:
    x: float
    y: float
    path: str
    z_index: float

    # These are subtracted from x, y for the in-game position
    x_offset: float  # Always 0.0?
    y_offset: float  # Always 0.0?


@dataclass
class Chunk:
    x: int
    y: int

    # Loaded status? Always True?
    # Rewriting a file to have False here instead didn't seem to do anything.
    loaded: bool


@dataclass
class StreamInfoFile:
    version: int
    seed: int
    frames_played: int
    seconds_played: float
    some_counter: int
    backgrounds: list[Background]

    schema_hash: str

    game_mode_nr: int
    game_mode_name: str
    game_mode_steam_id: int

    non_nolla_mod_active: bool

    # This appears to be in local time
    # Year, month, day, hour, minute, seconds
    save_and_quit_time: tuple[int, int, int, int, int, int]

    ui_newgame_name: str

    # Something player/coord related..
    # First two numbers sort of follow the player pos
    # Second two are always (?) 559, 370
    camera: tuple[int, int, int, int]

    chunks: list[Chunk]


def read_string(read_stream: typing.BinaryIO):
    (length,) = struct.unpack(">i", read_stream.read(4))
    return read_stream.read(length).decode("utf-8")


def read_stream_info_file(read_stream: typing.BinaryIO):
    (version,) = struct.unpack(">i", read_stream.read(4))

    if version != 24:
        raise Exception(f"Unsupported .stream_info version {version} expected 24.")

    (seed,) = struct.unpack(">I", read_stream.read(4))
    (frames_played,) = struct.unpack(">i", read_stream.read(4))
    (seconds_played,) = struct.unpack(">f", read_stream.read(4))
    (some_counter,) = struct.unpack(">Q", read_stream.read(8))

    backgrounds = read_backgrounds(read_stream)

    (chunk_count,) = struct.unpack(">i", read_stream.read(4))

    schema_hash = read_string(read_stream)

    (game_mode_nr,) = struct.unpack(">i", read_stream.read(4))
    game_mode_name = read_string(read_stream)
    (game_mode_steam_id,) = struct.unpack(">Q", read_stream.read(8))

    (non_nolla_mod_active,) = struct.unpack("?", read_stream.read(1))

    save_and_quit_time = struct.unpack(">HHHHHH", read_stream.read(12))

    ui_newgame_name = read_string(read_stream)

    camera = struct.unpack(">iiii", read_stream.read(16))

    chunks = [read_chunk(read_stream) for _ in range(chunk_count)]

    return StreamInfoFile(
        version=version,
        seed=seed,
        frames_played=frames_played,
        seconds_played=seconds_played,
        some_counter=some_counter,
        backgrounds=backgrounds,
        schema_hash=schema_hash,
        game_mode_nr=game_mode_nr,
        game_mode_name=game_mode_name,
        game_mode_steam_id=game_mode_steam_id,
        non_nolla_mod_active=non_nolla_mod_active,
        save_and_quit_time=save_and_quit_time,
        ui_newgame_name=ui_newgame_name,
        camera=camera,
        chunks=chunks,
    )


def read_backgrounds(read_stream: typing.BinaryIO) -> list[Background]:
    (background_count,) = struct.unpack(">I", read_stream.read(4))
    return [read_background(read_stream) for _ in range(background_count)]


def read_background(read_stream: typing.BinaryIO) -> Background:
    x, y = struct.unpack(">ff", read_stream.read(8))
    path = read_string(read_stream)
    (z_index, x_offset, y_offset) = struct.unpack(">fff", read_stream.read(12))
    return Background(
        x=x,
        y=y,
        path=path,
        z_index=z_index,
        x_offset=x_offset,
        y_offset=y_offset,
    )


def read_chunk(read_stream: typing.BinaryIO) -> Chunk:
    x, y, loaded = struct.unpack(">ii?", read_stream.read(9))
    return Chunk(x=x, y=y, loaded=loaded)


if __name__ == "__main__":
    print(read_stream_info_file(sys.stdin.buffer))
