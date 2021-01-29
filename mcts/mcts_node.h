#pragma once

#include <memory>
#include <vector>

#include "core/chess_board.h"
#include "mcts/mcts_rollout.h"

#include "net/board_tensor.h"
#include "net/data_encoder.h"

#include <fstream>

//contains a chessboard and an array of branches (positions that can be traversed) also holds a ucb1 score

namespace mcts
{
class Node
{
public:
    Node(ChessBoard cb, Node* parent = nullptr, int id = 0);
    Node(ChessBoard cb, ChessMove move, Node* parent = nullptr, int id = 0);

    inline ChessBoard get_board() const {return cb_;}

//    void propagate_score_update(const double & score, int status);
    void propagate_score_update();
    void propagate_score_update(const float & score);

    inline void add_child(const ChessBoard cb) {childs_.emplace_back(std::make_unique<Node>(cb, this));}
    inline void add_child(const ChessBoard cb, ChessMove move) {childs_.emplace_back(std::make_unique<Node>(cb, move, this));}

//    inline void add_child(std::unique_ptr<Node> child) {childs_.emplace_back(std::move(child));}
    inline Node* get_child(const int & c_idx) { return childs_.at(c_idx).get();}
    inline Node* get_parent() {return parent_;}

    double ucb1_score(); //used for deciding path in tree search
    double total_score(); //used in final result to eval node strength

    inline int get_visits() const { return visits_; }
    inline int get_depth() const { return depth_; }

    //ques ucb0 scores for each branch, selects highest/lowest scorer based on if search node is black or white
    int min_max() const;

    inline bool is_leaf() const {return childs_.empty(); }
    inline int get_n_childs() const { return childs_.size(); }

    void debug_populate_ucbvec();
    void debug_print_childucb0();

    void debug_print_child_totalscore();
    inline int get_id() const { return id_;}

    //after tree is constructed , calls this on root to recursive select output and write it to file
    void nn_log(const int &n_top, const int &n_bottom, DataEncoder& encoder_, std::ofstream &node_data);

    ChessBoard cb_;

    int color() const;
    void setIs_white_node(int is_white_node);

    inline void set_status(const int & status) { status_ = status;}
    int get_status() const { return status_; }

    inline ChessMove get_move() const { return move_; }
private:
    int depth_ = 0;
    double score_ = 0;
    int visits_ = 0;
    int id_;
    int is_white_node_ = 1;
    int status_ = 0;

    Node* parent_ = nullptr;
    std::vector<std::unique_ptr<Node>> childs_;

    ChessMove move_; // what move let to this position
};
}

