#pragma once

#include <memory>

#include "autoplay/board_wrapper.h"


//board wrapper input, now is fucked up because we can only have one instance of move generator. i gotta fix the static instancing of all arrays so we can do multiple move gens without reiniting all stuff

//this can provoke a random playout from any mcts node position


namespace mcts
{
class Node;

class Rollout
{
public:
    Rollout(MoveGenerator* move_gen);

    //plays out the game and automatically calls backward prop of score on the node input
    void perform_rollout(Node* node);

    void expand_node(Node* node);

private:
    std::unique_ptr<BoardWrapper> env_;
};
}

