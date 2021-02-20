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
    void start_search(int max_entries, int requested_rollouts =-1);
    void start_search_full(int fill_depth, int rollouts_per_branch);

    void extract_node_nn_data(Node* node, std::ofstream & node_data);

    void log_data(std::string filepath);

    int get_entries() const;

    void allocate_branch_expand_on_threads(std::vector<Node *> node_vec, int rollouts_per_branch);

private:

    MoveGenerator* move_gen_ = nullptr;

    std::unique_ptr<Node> root_ = nullptr;
    std::unique_ptr<Rollout> rollout_;

    std::map<int, int> depth_entries_;
    int m_depth_ = 10;
    int n_entries = 0;

    DataEncoder encoder_;

};
}

