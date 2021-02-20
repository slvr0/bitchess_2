#pragma once

#include <iostream>

#include "core/move_generator.h"
#include "utils/global_utils.cpp"


void apply_move_and_count(MoveGenerator &move_gen, int & total_moves, ChessBoard cb, int & depth, const int& max_depth, Timer &t, float &time);
void move_gen_test_ply(const int & max_depth);
