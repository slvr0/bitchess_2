#pragma once

#include <memory>
#include <map>

#include "core/move_generator.h"
#include "mcts/mcts_node.h"
#include "mcts/mcts_rollout.h"
#include "net/data_encoder.h"
#include "net/mqtt_client.h"
#include "mcts/cached_positions.h"
#include "utils/global_utils.cpp"

namespace mcts
{
//sets up the environment and start filling the monte carlo search tree with the logic of the algorithm

class TreeSearch
{
public:
    TreeSearch(MoveGenerator* move_gen, int folder_id, int max_entries, int requested_rollouts,  NetCachedPositions* cached_positions, MQTT_PIPE * mqtt_query_cache_pipe , MQTT_PIPE* mqtt_init_finish_pipe);

    void init_tree(const ChessBoard & start_position);
    void clear_tree();

    void start_search();

    void start_search_full(int fill_depth, int rollouts_per_branch);

    void extract_node_nn_data(Node* node, std::ofstream & node_data);

    void log_data(std::string filepath);

    void finish_search_and_publish_best_move();

    int get_entries() const;

    void allocate_branch_expand_on_threads(std::vector<Node *> node_vec, int rollouts_per_branch);

    void status_tree() const;

    bool get_is_init() const;
    void set_is_init(bool is_init);

private:

    MQTT_PIPE * mqtt_query_cache_pipe_;
    MQTT_PIPE* mqtt_init_finish_pipe_;

    MoveGenerator* move_gen_ = nullptr;

    NetCachedPositions* cached_positions_;

    std::unique_ptr<Node> root_ = nullptr;
    std::unique_ptr<Rollout> rollout_;

    std::map<int, int> depth_entries_;

    DataEncoder encoder_;

    int n_nonleaf_traversal_;
    int folder_id_;

    int total_entries_;
    int max_entries_;

    int total_rollouts_;
    int max_rollouts_;

    //checkpoint status for current search
    int chkpt_ = 0;
    int chkpt_ct_ = 0;
    bool is_queuing_search_;

    Timer t0_;

    bool is_init_;



};
}

