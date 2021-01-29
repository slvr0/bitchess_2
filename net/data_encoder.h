#pragma once

#include "core/chess_board.h"
#include "core/chess_move.h"

#include "utils/global_utils.cpp"

#include "net/board_tensor.h"

//converts data from chessboard, node or move to network input

namespace mcts
{
    class Node;
}


struct MctsNodeData
{
    BoardTensor bt_;

    std::vector<double> logits_; // branch values
    std::vector<double> logits_idcs_;
    double value_; //node value
};

class DataEncoder
{
public:
    DataEncoder();

    BoardTensor board_as_tensor(const ChessBoard & cb);
    int move_as_nn_input(const ChessMove & move);

   MctsNodeData node_as_nn_input(mcts::Node* node);

    std::vector<int> movelist_as_nn_mask(const ChessMoveList & movelist);

    inline void  fill_tensor (BoardTensor& bt, int type, const uint64_t& x)
    {
        bt.set(type, x) ;
    }
private:

    int input_dims0_ = 13;
    int input_dims1_ = 64;

    int output_dims_;

};
