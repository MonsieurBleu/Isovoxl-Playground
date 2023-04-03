#ifndef COORDS_HPP
#define COORDS_HPP

#include <constants.hpp>

struct block_coordonate;

struct coord3D
{
    int x;
    int y;
    int z;

    bool operator==(const coord3D& other) const;
    coord3D operator+(const coord3D& other) const;

    block_coordonate to_block_coordonate() const;
};



typedef coord3D chunk_coordonate;

typedef coord3D world_coordonate;

struct block_coordonate : coord3D
{
    chunk_coordonate chunk;


    block_coordonate() : coord3D{0, 0, 0}, chunk{0, 0, 0} {}
    block_coordonate(int x, int y, int z, chunk_coordonate chunk) : coord3D{x, y, z}, chunk{chunk} {}
    block_coordonate(const block_coordonate& other) : coord3D{other.x, other.y, other.z}, chunk{other.chunk} {}

    bool operator==(const block_coordonate& other) const;
    
    bool operator<(const block_coordonate& other) const;

    block_coordonate operator+(const coord3D& other) const;

    coord3D to_coord3D() const;
};


struct coord2D
{
    int x;
    int y;
};

typedef coord2D pixel_coord;

struct fcoord3D
{
    float x;
    float y;
    float z;
};

struct fcoord2D
{
    float x;
    float y;
};


std::ostream& operator<<(std::ostream& os, const coord3D& coord);

std::ostream& operator<<(std::ostream& os, const block_coordonate& coord);
#endif