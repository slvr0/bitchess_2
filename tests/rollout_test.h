#pragma once

#include "core/chess_board.h"
#include "autoplay/board_wrapper.h"
#include "core/move_generator.h"

class RolloutPhaseTest
{
public:
    RolloutPhaseTest();

    float test_rollouts(std::string position, const int & n_simuls, const int& n_steps);

private:
    MoveGenerator move_gen_;
    BoardWrapper env_{&move_gen_};

};
