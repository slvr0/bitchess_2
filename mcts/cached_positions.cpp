#include "cached_positions.h"

#include <iostream>

NetCachedPositions::NetCachedPositions() :
    cached_positions_m()
{

}

NetCachedPositions::NetCachedPositions(CachedPositionsMap *stored_positions) :
    cached_positions_m(*stored_positions)
{


}

int NetCachedPositions::entries() const
{
    return cached_positions_m.size();
}

std::map<int, float> NetCachedPositions::get(const uint64_t &zobrist_hash_key) const
{ 

    if(exist(zobrist_hash_key))  return cached_positions_m.at(zobrist_hash_key);
    else
    {
        std::cout << "element not found in NetCachedPositions : " << zobrist_hash_key;
        return std::map<int, float> ();
    }
}

std::map<int, float> NetCachedPositions::get(const ChessBoard & cb) const
{
    return get(cb.get_zobrist());
}

void NetCachedPositions::del(const uint64_t &zobrist_hash_key)
{
    if(exist(zobrist_hash_key))
    {
            auto element = cached_positions_m.find(zobrist_hash_key);
            cached_positions_m.erase (element);
    }
}

void NetCachedPositions::add(const uint64_t &zobrist_hash_key, std::map<int, float> nn_logs)
{
//    std::cout << zobrist_hash_key;

    if(!exist(zobrist_hash_key)) cached_positions_m[zobrist_hash_key] = nn_logs;
}

bool NetCachedPositions::exist(std::string fen_position) const
{
    return exist(ChessBoard(fen_position));
}

bool NetCachedPositions::exist(const ChessBoard &cb) const
{  
    return cached_positions_m.find(cb.get_zobrist()) != cached_positions_m.end();
}

bool NetCachedPositions::exist(const uint64_t &zobrist_hash_key) const
{
    return cached_positions_m.find(zobrist_hash_key) != cached_positions_m.end();
}

void NetCachedPositions::flush()
{
    cached_positions_m = CachedPositionsMap();
}
