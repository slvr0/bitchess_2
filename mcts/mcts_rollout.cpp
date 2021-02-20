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
    srand((unsigned) time(0));
}

void Rollout::perform_rollout(Node *node)
{
    int m_steps = 100;
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

    node->propagate_score_update(score);
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

std::vector<Node*> Rollout::expand_and_rollout_node(Node* node, int rollout_at_depth, int n_rollouts, int & total_rollouts)
{
    env_->reset(node->get_board());

    auto moves = env_->get_actions();

    for(int idx = 0 ; idx < moves.size(); ++idx)
    {
        ChessBoard branch = env_->explore(moves.get(idx));

        node->add_child(branch, moves.get(idx));
    }

//    if(node->get_depth() == rollout_at_depth - 1)
//    {
//        int n_childs = node->get_n_childs();

//        total_rollouts += int(n_childs * n_rollouts);

//        for(int i = 0 ; i < node->get_n_childs() ; ++i)
//        {
//            for(int k = 0 ; k < n_rollouts; ++k)  this->perform_rollout(node->get_child(i));

//            //std::cout << int(progress_s * i) << " % ";
//        }
//        //std::cout << std::endl;
//    }

    return node->get_childs();
}

void Rollout::thread_rollout(MoveGenerator* move_gen, std::vector<Node *> nodelist, const int & thread_id, const int &N, const int &threads, const int &n_rollouts)
{

    std::unique_ptr<BoardWrapper> env =  std::make_unique<BoardWrapper> (move_gen);

   int search_per_thread = int(N / threads);

    srand((unsigned) time(0));

    int range_start = thread_id * search_per_thread;
    int range_end = ( thread_id + 1) * search_per_thread;
    if(range_end >= N) range_end = N;

//    std::cout << "id : " << thread_id << "start : " << range_start << " end : " << range_end << std::endl;

    for(int i =range_start ; i < range_end; ++i)
    {
        if(i >= N) break;

        Node* node = nodelist.at(i);

        for(int k = 0 ; k < n_rollouts; ++k)
        {
            int m_steps = 100;
            double score = 0;

            env->reset(node->get_board());

            BoardInfo info;

            for(int i = 0 ; i < m_steps; ++i)
            {

                auto moves = env->get_actions();


                info = env->get_info();

                score = info.reward;

                if(info.done )
                {
                    break;
                }

                auto rm = rand() % moves.size();

                env->step(rm);
            }
            //node->set_status(info.status);

            node->propagate_score_update(score);
        }


    }

    //log
    std::string filepath = "training_data/thread_" + std::to_string(thread_id) + "_data_.txt";
    std::ofstream thread_file_log(filepath);
    DataEncoder encoder_;


    if(thread_id == 0 ) //log root on thread 0
    {
        Node* root =  nodelist.at(0)->get_parent();

        while(true)
        {
            if(!root->get_parent()) break;

            root = root->get_parent();
        }

        root->nn_log_norecursive(encoder_, thread_file_log);
    }

    for(int i =range_start ; i < range_end; ++i)
    {
         Node* node = nodelist.at(i);
         node->nn_thread_log(encoder_, thread_file_log);

    }



}


