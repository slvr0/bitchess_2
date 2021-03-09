#include "mcts_search.h"

#include <iostream>
#include <fstream>
#include <thread>
#include "mcts/mcts_node.h"
#include "utils/global_utils.cpp"

using namespace mcts;

TreeSearch::TreeSearch(MoveGenerator* move_gen, int folder_id, int max_entries, int requested_rollouts,  NetCachedPositions* cached_positions, MQTT_PIPE *mqtt_query_cache_pipe) :
    move_gen_(move_gen),
    folder_id_(folder_id),
    max_entries_(max_entries),
    max_rollouts_(requested_rollouts),
    rollout_(std::make_unique<Rollout> (move_gen)),
    cached_positions_(cached_positions),
    mqtt_query_cache_pipe_(mqtt_query_cache_pipe)

{
    for(int i = 0; i < 10; ++i)
    {
        depth_entries_[i] = 0;
    }

    total_entries_ = 0;
    total_rollouts_ = 0;

    srand((unsigned) time(0));
}

void TreeSearch::init_tree(const ChessBoard &start_position)
{
    if(root_)
        clear_tree();

    root_ = std::make_unique<Node> (start_position, nullptr);

    total_entries_ = 1;
    total_rollouts_ = 0;
    t0_ = Timer();
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

void TreeSearch::status_tree() const
{
    std::cout << " number of entries : " << total_entries_ << " -- total number of rollouts : " << total_rollouts_ << std::endl;

    for(int i = 0; i < 10 ; ++i)
    {
        std::cout << "entries at depth : [" << i << "] = " << depth_entries_.at(i) << std::endl;
    }

    std::cout.precision(1);
    std::cout << "time elapsed building tree : " << t0_.elapsed() << " seconds" << std::endl;

    std::cout.precision(8);

    root_->cb_.print_to_console();
    root_->debug_print_child_totalscore();
}

void TreeSearch::start_search()
{
    Node* current_ = nullptr;

    int chkpt = 0;
    int chkpt_ct = 0;

    depth_entries_[0] = 1;

    root_->cb_.print_to_console();

    if(max_rollouts_ == -1 ) max_rollouts_ = max_entries_;

    while(total_entries_ < max_entries_ || total_rollouts_ < max_rollouts_)
    {
        current_= root_.get();

        //when tree is full, simply want to rollout randomly
        if(total_entries_ >= max_entries_)
        {
            while(!current_->is_leaf())
            {
                int bscore_child = current_->min_max();

                current_ = current_->get_child(bscore_child);
            }

            rollout_->perform_rollout(current_);
            total_rollouts_++;

            if(total_rollouts_ % 100000 == 0) std::cout << "Rollouts : " << total_rollouts_ << std::endl;

            continue;
        }

        if(total_entries_ > chkpt)
        {
            std::cout << "%["  << chkpt_ct <<  "] " << std::endl;
            chkpt_ct += 10;
            chkpt += int(max_entries_ /10);
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
            total_entries_++;
        }
        else
        {
            //expand it!

            if(!cached_positions_->exist(current_->cb_))
            {
                mqtt_query_cache_pipe_->publish_message(current_->cb_.fen());
                return;
            }

            std::map<int, float> nn_exp_data = cached_positions_->get(current_->cb_.get_zobrist());

            rollout_->expand_node(current_, nn_exp_data , folder_id_);

            total_entries_ += current_->get_n_childs();

            depth_entries_.at(current_->get_depth() + 1) += current_->get_n_childs();

            if(!current_->is_leaf())
            {
                int bscore_child = current_->min_max();

                current_ = current_->get_child(bscore_child);

                rollout_->perform_rollout(current_);

                total_rollouts_++;
            }
            else
            {
                current_->propagate_score_update(current_->total_score());
            }
        }
    }

    int  winner = get_best_move();

    //evaluate best move, send it on mcts_tree_finish!;

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

    while(current_depth < fill_depth)
    {

        std::vector<Node*> depth_nodes_new;
        //fill current depth

        int N = depth_nodes.size();
        int n = 0;
        for(auto & node : depth_nodes)
        {
              auto new_branches = rollout_->expand_and_rollout_node(node, fill_depth, rollouts_per_branch, total_rollouts, folder_id_);

              total_entries_ += new_branches.size();

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
    int N = total_entries_;
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

int TreeSearch::get_best_move() const
{
    print("about to publish best move!!!");
    return 0;
}

int TreeSearch::get_entries() const
{
    return total_entries_;
}




