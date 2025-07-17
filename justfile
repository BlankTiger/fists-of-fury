default:
    @just --list

config:
    cmake -S . -B build

config-ninja:
    cmake -G Ninja -B build

build:
    cmake --build build --parallel $(nproc)
