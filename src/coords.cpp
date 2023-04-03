#include <coords.hpp>
#include <constants.hpp>
#include <iostream>

bool block_coordonate::operator==(const block_coordonate& other) const {
    return x == other.x && y == other.y && z == other.z && chunk.x == other.chunk.x && chunk.y == other.chunk.y && chunk.z == other.chunk.z;
}


bool coord3D::operator==(const coord3D& other) const {
    return x == other.x && y == other.y && z == other.z;
}

coord3D coord3D::operator+(const coord3D& other) const {
    coord3D result;
    result.x = x + other.x;
    result.y = y + other.y;
    result.z = z + other.z;
    return result;
}

block_coordonate coord3D::to_block_coordonate() const {
    block_coordonate result;
    result.x = x % CHUNK_SIZE;
    result.y = y % CHUNK_SIZE;
    result.z = z % CHUNK_SIZE;
    result.chunk.x = x / CHUNK_SIZE;
    result.chunk.y = y / CHUNK_SIZE;
    result.chunk.z = z / CHUNK_SIZE;
    return result;
}

std::ostream& operator<<(std::ostream& os, const coord3D& coord) {
    os << "(" << coord.x << ", " << coord.y << ", " << coord.z << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const block_coordonate& coord) {
    os << "(" << coord.x << ", " << coord.y << ", " << coord.z << ") in chunk (" << coord.chunk.x << ", " << coord.chunk.y << ", " << coord.chunk.z << ")";
    return os;
}

coord3D block_coordonate::to_coord3D() const {
    coord3D result;
    result.x = x + chunk.x * CHUNK_SIZE;
    result.y = y + chunk.y * CHUNK_SIZE;
    result.z = z + chunk.z * CHUNK_SIZE;
    return result;
}