# NoitaSettings

Simple program that can dump Noita's `mod_settings.bin` file.

I mainly put this here as a reference in case anyone needs to know the
format of this file.

It's a simple binary format, the only tricky part is that you have to
know the file is compressed using
[FastLZ](https://github.com/ariya/FastLZ.git).
