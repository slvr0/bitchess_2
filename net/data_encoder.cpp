#include "data_encoder.h"

#include "utils/global_utils.cpp"
#include "utils/chess_enums.cpp"

#include "mcts/mcts_node.h"

#include <stdio.h>
#include <ctype.h>
#include <algorithm>

DataEncoder::DataEncoder()
{
    output_dims_ = sizeof(nn_action_space_str)/sizeof(nn_action_space_str[0]);
}

BoardTensor DataEncoder::board_as_tensor(const ChessBoard &cb) const
{
   BoardTensor bt;

    fill_tensor(bt, 0, cb.get_pawns());
    fill_tensor(bt, 1,cb.get_knights());
    fill_tensor(bt, 2,cb.get_bishops());
    fill_tensor(bt, 3,cb.get_rooks());
    fill_tensor(bt, 4, cb.get_queens());
    fill_tensor(bt, 5,cb.get_king());
    fill_tensor(bt, 6,cb.get_enemy_pawns());
    fill_tensor(bt, 7,cb.get_enemy_knights());
    fill_tensor(bt, 8,cb.get_enemy_bishops());
    fill_tensor(bt, 9,cb.get_enemy_rooks());
    fill_tensor(bt, 10,cb.get_enemy_queens());
    fill_tensor(bt, 11, cb.get_enemy_king());

    auto we00 = cb.castling_.getWe_00();
    auto we000 = cb.castling_.getWe_000();
    auto enemy00 = cb.castling_.getEnemy_00();
    auto enemy000 = cb.castling_.getEnemy_000();

    uint64_t castle_64{0x0};

    if(we00) castle_64 |= 1ULL << 1;
    if(we000)  castle_64 |= 1ULL << 2;
    if(enemy00)  castle_64 |= 1ULL << 3;
    if(enemy000)  castle_64 |= 1ULL << 4;
    if(cb.get_whitetoact())  castle_64 |= 1ULL << 5;

    fill_tensor(bt, 12, castle_64);

    return bt;
}

int DataEncoder::move_as_nn_input(const ChessMove &move) const
{
    int from = move.from();
    int to = move.to();
    char promo = move.promotion();
    std::string s;

    auto f_string = board_notations[from];
    std::for_each(f_string.begin(), f_string.end(), [](char & c) {
           c = tolower(c);
       });

    auto t_string = board_notations[to];
    std::for_each(t_string.begin(), t_string.end(), [](char & c) {
           c = tolower(c);
       });

    if(promo != ' ')
    {
        s = f_string + t_string;
        s.insert(4, 1, tolower(promo));
    }
    else
    {
        s = f_string + t_string;
    }
    auto action_id = std::find(nn_action_space_str, nn_action_space_str + output_dims_, s);

    if(action_id != std::end(nn_action_space_str))
    {
        int action_pos = std::distance(nn_action_space_str, action_id);

        return std::distance(nn_action_space_str, action_id);
    }
    else
    {
        print("element not found in action space");
    }
}

MctsNodeData DataEncoder::node_as_nn_input(const mcts::Node& node) const
{
    const ChessBoard cb = node.get_board();
    BoardTensor bt = board_as_tensor(cb);

    //get all branch values

    MctsNodeData node_data;

    for(int i=0; i<node.get_n_childs();++i)
    {
        auto move = node.get_child(i)->get_move();
        auto branch_score =  node.get_child(i)->total_score();
        auto branch_nn_index =  move_as_nn_input(move);

        node_data.logits_.emplace_back(branch_score);
        node_data.logits_idcs_.emplace_back(branch_nn_index);
    }

    node_data.value_ = node.total_score();
    node_data.bt_ = bt;

    return node_data;
}

std::vector<int> DataEncoder::movelist_as_nn_mask(const ChessMoveList &movelist)
{

}
