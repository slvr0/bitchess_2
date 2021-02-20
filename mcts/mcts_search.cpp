#include "mcts_search.h"

#include <iostream>
#include <fstream>
#include <thread>

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

    srand((unsigned) time(0));
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

void TreeSearch::allocate_branch_expand_on_threads(std::vector<Node *> node_vec, int rollouts_per_branch)
{
    std::unique_ptr<MoveGenerator> move_gen =std::make_unique<MoveGenerator> ();
    std::vector<std::thread> vth;

    int N = node_vec.size();
    std::cout << "branches : " << N << std::endl;
    int threads = 5 ;

     for(int n = 0; n < threads ; ++n)
     {
//         void Rollout::thread_rollout(std::vector<Node *> nodelist, const int & thread_id, const int &N, const int &threads, const int &n_rollouts)
         vth.emplace_back(std::thread(mcts::Rollout::thread_rollout, move_gen.get(), node_vec, n, N, threads, rollouts_per_branch));
     }

     for(auto & thread : vth) thread.join();
     vth.clear();

     std::cout << "performed " << N*rollouts_per_branch << " rollouts " << std::endl;
}

void TreeSearch::start_search_full(int fill_depth, int rollouts_per_branch)
{
    int t_rollouts = 0;
    int t_entries = 1;

    Timer t0;

    int current_depth = 0;

     depth_entries_[0] = 1;

    std::vector<Node*> depth_nodes;
    depth_nodes.emplace_back(root_.get());
    int total_rollouts = 0;

    while(current_depth < fill_depth + 1)
    {

        std::vector<Node*> depth_nodes_new;
        //fill current depth

        int N = depth_nodes.size();
        int n = 0;
        for(auto & node : depth_nodes)
        {
              auto new_branches = rollout_->expand_and_rollout_node(node, fill_depth, rollouts_per_branch, total_rollouts);

//              if(current_depth == fill_depth)
//              {
//                  std::cout << "number of rollouts : " << total_rollouts <<  " progress : " << int(100.f / N * n) << " % " <<  std::endl;
//              }
//              ++n;

              n_entries += new_branches.size();

              std::copy(new_branches.begin(), new_branches.end(), std::back_inserter(depth_nodes_new));
        }

        depth_nodes.clear();
        std::copy(depth_nodes_new.begin(), depth_nodes_new.end(), std::back_inserter(depth_nodes));

        depth_entries_[current_depth] = depth_nodes_new.size();
        depth_nodes_new.clear();

        current_depth++;
    }

    allocate_branch_expand_on_threads(depth_nodes, rollouts_per_branch); //wtf is this method name



    for(int i = 0; i < 10 ; ++i)
    {
        std::cout << "entries at depth : [" << i << "] = " << depth_entries_.at(i) << std::endl;
    }

    std::cout.precision(1);
    std::cout << "time elapsed building tree : " << t0.elapsed() << " seconds" << std::endl;

    root_->debug_print_child_totalscore();
}

void TreeSearch::start_search(int max_entries, int requested_rollouts)
{
    int t_rollouts = 0;
    int t_entries = 1;
    Node* current_ = nullptr;

    int chkpt = 0;
    int chkpt_ct = 0;

    Timer t0;    

    depth_entries_[0] = 1;

    if(requested_rollouts == -1 ) requested_rollouts = max_entries;


    int tree_max_depth = 0;

    while(t_entries < max_entries || t_rollouts < requested_rollouts)
    {
        current_= root_.get();

        //when tree is full, simply want to rollout randomly
        if(t_entries >= max_entries)
        {
//            int at_depth = rand() % tree_max_depth - 1;
//            at_depth += 1;

            while(!current_->is_leaf())
            {
                int bscore_child = current_->min_max();

                current_ = current_->get_child(bscore_child);
            }


            rollout_->perform_rollout(current_);
            t_rollouts++;

            if(t_rollouts % 100000 == 0)
            {
                print(t_rollouts);
            }


            continue;
        }

        if(t_entries > chkpt)
        {
            std::cout << "%["  << chkpt_ct <<  "] " << std::endl;
            chkpt_ct += 10;
            chkpt += int(max_entries /10);
        }

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
                current_->propagate_score_update(current_->total_score());
            }

            if(current_->get_depth() > tree_max_depth)
            {
                tree_max_depth = current_->get_depth();
            }
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

    n_entries = t_entries;
}

void TreeSearch::extract_node_nn_data(Node *node, std::ofstream & node_data)
{
    MctsNodeData nn_data = encoder_.node_as_nn_input((*node));

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
    root_->debug_print_child_totalscore();

    //create vector of files
    int N = n_entries;
    int entries_per_file = 1000;
    int n_files = int(N/entries_per_file);

    print(n_files);

    std::vector<std::ofstream*> node_datas;
    for(int i = 0; i < n_files + 1; ++i)
    {
        std::string filepath = "training_data/thread_" + std::to_string(0) + "_data_" + std::to_string(i) + ".txt";
        node_datas.emplace_back(new std::ofstream (filepath));
    }

    int entry = 0;
    root_->nn_log(1,1,encoder_, node_datas, entries_per_file, entry);

    for(auto & filestream : node_datas)
    {
        filestream->close();
    }
}

int TreeSearch::get_entries() const
{
    return n_entries;
}




