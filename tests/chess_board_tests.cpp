#pragma once

#include "chess_board.h"

void run_chessboard_test()
{
    auto fen_pos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    ChessBoard cb(fen_pos);

    cb.print_to_console();




}
