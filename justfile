default:
    @just --list

config:
    cmake -S . -B build

build:
    cmake --build build
