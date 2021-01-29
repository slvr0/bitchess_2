#include "mcts_search.h"

#include <iostream>
#include <fstream>

#include "utils/global_utils.cpp"

using namespace mcts;

TreeSearch::TreeSearch(MoveGenerator* move_gen) :
    move_gen_(move_gen),
    rollout_(std::make_unique<Rollout> (move_gen))
{
    for(int i = 0; i < m_depth_; ++i)
    {
        depth_entries_[i] = 0;
    }
}

void TreeSearch::init_tree(const ChessBoard &start_position)
{
    if(root_)
        clear_tree();

    root_ = std::make_unique<Node> (start_position, nullptr);
}


void TreeSearch::clear_tree()
{
    root_ = nullptr; //uhoh
}

void TreeSearch::start_search(int max_entries)
{
    int t_rollouts = 0;
    int t_entries = 1;
    Node* current_ = nullptr;

    int max_ended_game_scans = 100000; // if we keep getting to a state that is terminal
    int endgame_scans = 0;

    int chkpt = 0;
    int chkpt_ct = 0;

    Timer t0;

    int loopcount = 0;

    depth_entries_[0] = 1;

    while(t_entries < max_entries)
    {

        if(t_entries > chkpt)
        {
            std::cout << "%["  << chkpt_ct <<  "] " << std::endl;
            chkpt_ct += 10;
            chkpt += int(max_entries /10);
        }

        current_= root_.get();

        while(!current_->is_leaf())
        {
            int bscore_child = current_->min_max();

            current_ = current_->get_child(bscore_child);
        }

        if( current_->get_visits() == 0 )
        {
            //explore it!            
            rollout_->perform_rollout(current_);
            t_rollouts++;
        }
        else
        {           
            //expand it!
            rollout_->expand_node(current_);

            t_entries += current_->get_n_childs();

            if(m_depth_ > current_->get_depth() + 1)
                depth_entries_.at(current_->get_depth() + 1) += current_->get_n_childs();

            //explore any branch( all are as good, we take the first )
            if(!current_->is_leaf())
            {
                current_ = current_->get_child(0);
                rollout_->perform_rollout(current_);
                t_rollouts++;
            }
            else
            {                
                current_->propagate_score_update();
            }

            ++loopcount;
        }
    }

    std::cout << " number of entries : " << t_entries << " -- total number of rollouts : " << t_rollouts << std::endl;

    for(int i = 0; i < 10 ; ++i)
    {
        std::cout << "entries at depth : [" << i << "] = " << depth_entries_.at(i) << std::endl;
    }

    std::cout.precision(1);
    std::cout << "time elapsed building tree : " << t0.elapsed() << " seconds" << std::endl;

    std::cout.precision(8);


}

void TreeSearch::extract_node_nn_data(Node *node, std::ofstream & node_data)
{
    MctsNodeData nn_data = encoder_.node_as_nn_input(node);    

    for(int i = 0 ; i < 13 ; ++i )
    {
        auto piece_vector = nn_data.bt_.get(i);

        auto piece_locations = bit_iter(piece_vector);

        for(const auto & loc : piece_locations)
        {
             node_data << int(loc) << " ";
        }

        node_data << "\n";
    }


    if(!nn_data.logits_.empty())
    {
        for(const auto & log : nn_data.logits_)
        {
            node_data << log << "  ";
        }

        node_data << "\n";

        for(const auto & log_idcs : nn_data.logits_idcs_)
        {
            node_data << log_idcs << "  ";
        }
    }
    else
    {
        node_data << "&\n&";
    }

    node_data << "\n";
    node_data << nn_data.value_;
    node_data << "\n";
}

void TreeSearch::log_data(std::string filepath)
{
    std::ofstream node_data(filepath);
    root_->nn_log(1,1,encoder_, node_data);
}


