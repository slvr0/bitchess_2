#pragma once

#include "core/chess_board.h"
#include "core/move_generator.h"


//status 0 = ongoing , 1 = white win, 2 = black win, 3 = draw, status = 4 stalemate
struct BoardInfo
{
public :

    int status = 0;
    int rule_50 = 0 ;
    int n_repeats = 0;
    int total_moves = 0;
    float reward = 0.f;
    bool done = false;
};

class BoardWrapper
{
public:
    BoardWrapper(MoveGenerator* move_gen);

    ChessMoveList get_actions(); //extract moves from the board
    BoardInfo get_info() const ; //extract all position information

    void step(ChessMove & move);
    void step(const int& move_idx);

    ChessBoard explore(const ChessMove &move);
    ChessBoard explore(const int & move_idx);

    void reset(const ChessBoard & cb);

    void show_state() const;

private :
    ChessBoard cb_;
    ChessMoveList movelist_;
    MoveGenerator* move_gen_;
    std::vector<uint64_t> zobrist_keys_; // we somehow gotta imititate the amazing collection Counter in python.
    bool has_queued_ = false; // if moves have been queued
    bool king_under_attack_ = false;
    int starting_color_white_ = 1; // used to see if position initiated from white. Decides reward sign output on draws
};
