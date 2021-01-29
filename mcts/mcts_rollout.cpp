#include "mcts_rollout.h"

#include <stdlib.h>
#include <time.h>

#include "mcts_node.h"
#include "core/chess_board.h"

#include "utils/global_utils.cpp"

using namespace mcts;

Rollout::Rollout(MoveGenerator* move_gen) :
    env_(std::make_unique<BoardWrapper> (move_gen))
{
    srand(time(0));
}

void Rollout::perform_rollout(Node *node)
{
    int m_steps = 150;
    double score = 0;

    env_->reset(node->get_board());

    //env_->show_state();

    BoardInfo info;

    for(int i = 0 ; i < m_steps; ++i)
    {
        auto moves = env_->get_actions();
        info = env_->get_info();

        score = info.reward;

        if(info.done )
        {
            break;
        }

        auto rm = rand() % moves.size();

        env_->step(rm);
    }

    node->set_status(info.status);
    node->propagate_score_update();
}

void Rollout::expand_node(Node *node)
{


    env_->reset(node->get_board());

    auto moves = env_->get_actions();

    for(int idx = 0 ; idx < moves.size(); ++idx)
    {
        ChessBoard branch = env_->explore(moves.get(idx));

        //std::unique_ptr<Node> n = std::make_unique<Node> (branch, moves.get(idx), node, idx);
        node->add_child(branch, moves.get(idx));
    }

}


