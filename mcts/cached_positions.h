#pragma once

#include <iostream>

#include <memory>
#include <map>
#include <vector>

#include <core/chess_board.h>

//position key is zobrist hash



class NetCachedPositions
{
    typedef std::map<uint64_t,std::map<int, float>> CachedPositionsMap;

public:
    NetCachedPositions();
    NetCachedPositions(CachedPositionsMap* stored_positions);

    int entries()const ;

    std::map<int, float> get(const uint64_t & zobrist_hash_key) const;
    void del(const uint64_t &zobrist_hash_key);
    void add(const uint64_t &zobrist_hash_key,  std::map<int, float> nn_logs);

    bool exist(const uint64_t & zobrist_hash_key) const;
    bool exist(std::string fen_position) const;
    bool exist(const ChessBoard& cb) const;

    void flush();

private :
    //fen string key, float is net logits
    CachedPositionsMap cached_positions_m;

};

