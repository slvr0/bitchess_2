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

    void expand_node(Node* node, std::vector<std::pair<int, float> > nn_expand_data, int folder_id);

    std::vector<Node*> expand_and_rollout_node(Node* node, int rollout_at_depth, int n_rollouts, int &total_rollouts, int folder_id);

    static void thread_rollout(MoveGenerator *move_gen, std::vector<Node*> nodelist, const int &thread_id, const int & N, const int & threads, const int &n_rollouts);


private:
    std::unique_ptr<BoardWrapper> env_;
};
}

