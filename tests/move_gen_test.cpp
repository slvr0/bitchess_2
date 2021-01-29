#pragma once

#include <iostream>

#include "core/move_generator.h"
#include "utils/global_utils.cpp"

void run_move_gen_test()
{
    auto fen_pos = "rnbqkbnr/pppppppp/8/8/1K6/8/1PPPPPPP/RNBQ1BNR w kq - 0 1";
    ChessBoard cb(fen_pos);

    MoveGenerator move_gen;
    auto moves = move_gen.get_legal_moves(cb);

//    Timer t0;
//    int iters = 100000;

//    for(int i = 0 ; i < iters ; ++i)
//    {
//
//    }

//    auto elapsed_time = t0.elapsed();

//    std::cout << "moves per second : " << 20 * iters / elapsed_time;
    //print(t0.elapsed());
    moves.print_moves();




}
