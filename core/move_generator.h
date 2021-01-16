#pragma once

#include <iostream>
#include <vector>

#include "core/chess_board.h"
#include "core/chess_move.h"

struct PushPinInfo
{

public:

    PushPinInfo() {}

    int n_attackers{0};
    uint64_t push_mask_full{0x0};
    uint64_t capture_mask{0x0};
    std::vector<std::pair<int, uint64_t>> pins;
};

class MoveGenerator
{
public:
    MoveGenerator();

    ChessMoveList get_legal_moves(const ChessBoard & cb);

private:
    typedef std::pair<int,int> Direction;
    std::vector<Direction> all_dirs {Direction(1,0), Direction(-1,0), Direction(0,1), Direction(0,-1),Direction(0,-1) ,Direction(1, 1), Direction(1,-1), Direction(-1,0), Direction(-1,-1)};

    inline bool on_board(const unsigned long & row, const unsigned long & col){return   row >= 0 && row < 8 && col >= 0 && col < 8;}

    uint64_t get_king_moves(unsigned long idx); //from an empty board
    PushPinInfo get_push_pin_info(const unsigned long& king, const uint64_t& our_pieces, const uint64_t& enemy_pieces,
                                  const  uint64_t& enemy_bishops,const uint64_t& enemy_rooks, const uint64_t&  enemy_queens);

    void append_pseudolegal_slidermoves(const ChessBoard& cb, unsigned long idx, ChessMoveList& movelist, char ptype);
    void append_pseudolegal_pawnmoves(const ChessBoard& cb,  unsigned long idx, ChessMoveList& movelist);

    bool spec_enp_check(const ChessBoard& cb, unsigned long capt_from);
};
