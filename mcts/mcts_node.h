#pragma once

#include <memory>
#include <vector>

#include "core/chess_board.h"
#include "mcts/mcts_rollout.h"

#include "net/board_tensor.h"
#include "net/data_encoder.h"

#include <fstream>

//contains a chessboard and an array of branches (positions that can be traversed) also holds a ucb1 score


namespace mcts{


class Node
{
public:
    Node(ChessBoard cb, Node* parent = nullptr, int folder_id = 0);
    Node(ChessBoard cb, ChessMove move, Node* parent = nullptr, int folder_id = 0);

    inline ChessBoard get_board() const {return cb_;}

    void propagate_score_update(const float & score);

    inline void add_child(const ChessBoard cb, int folder_id) {childs_.emplace_back(std::make_unique<Node>(cb, this, folder_id));}
    inline void add_child(const ChessBoard cb, ChessMove move, int folder_id) {childs_.emplace_back(std::make_unique<Node>(cb, move, this, folder_id));}

//    inline void add_child(std::unique_ptr<Node> child) {childs_.emplace_back(std::move(child));}
    inline Node* get_child(const int & c_idx) const { return childs_.at(c_idx).get();}
    inline Node* get_parent() const {return parent_;}

    double ucb1_score(); //used for deciding path in tree search
    double total_score() const; //used in final result to eval node strength

    inline int get_visits() const { return visits_; }
    inline int get_depth() const { return depth_; }

    //ques ucb0 scores for each branch, selects highest/lowest scorer based on if search node is black or white
    int min_max() const;

    inline bool is_leaf() const {return childs_.empty(); }
    inline int get_n_childs() const { return childs_.size(); }

    void debug_populate_ucbvec();
    void debug_print_childucb0();

    void debug_print_child_totalscore();
    inline int get_id() const { return folder_id_;}

    void nn_thread_log(DataEncoder& encoder_, std::ofstream & file);
    void nn_log_norecursive(DataEncoder& encoder_, std::ofstream & file);

    //after tree is constructed , calls this on root to recursive select output and write it to file
    void nn_log(const int &n_top, const int &n_bottom, DataEncoder& encoder_, std::vector<std::ofstream *> vector_files, int entries_per_file, int &entry);

    ChessBoard cb_;

    int color() const;
    void setIs_white_node(int is_white_node);

    inline void set_status(const int & status) { status_ = status;}
    int get_status() const { return status_; }

    inline std::vector<Node*> get_childs() const {
        std::vector<Node*> childs;

        for(const auto & child : childs_) childs.emplace_back(child.get());

        return childs;
    }

    std::pair<std::vector<float>, std::vector<int> > get_logits_idc_pair() const;


    inline ChessMove get_move() const { return move_; }
private:
    int depth_ = 0;
    double score_ = 0;
    int visits_ = 0;
    int folder_id_;
    int is_white_node_ = 1;
    int status_ = 0;

    Node* parent_ = nullptr;
    std::vector<std::unique_ptr<Node>> childs_;

    ChessMove move_; // what move let to this position

    bool logged_ = false;



};
}

