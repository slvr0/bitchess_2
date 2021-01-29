#pragma once

#include <memory>
#include <map>

#include "core/move_generator.h"
#include "mcts/mcts_node.h"
#include "mcts/mcts_rollout.h"
#include "net/data_encoder.h"

namespace mcts
{
//sets up the environment and start filling the monte carlo search tree with the logic of the algorithm

class TreeSearch
{
public:
    TreeSearch(MoveGenerator* move_gen);

    void init_tree(const ChessBoard & start_position);
    void clear_tree();
    void start_search(int max_entries);

    void extract_node_nn_data(Node* node, std::ofstream & node_data);

    void log_data(std::string filepath);

private:

    MoveGenerator* move_gen_ = nullptr;

    std::unique_ptr<Node> root_ = nullptr;
    std::unique_ptr<Rollout> rollout_;

    std::map<int, int> depth_entries_;
    int m_depth_ = 10;

    DataEncoder encoder_;

};
}

