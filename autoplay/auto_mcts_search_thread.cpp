#include "auto_mcts_search_thread.h"

#include <memory>
#include <unistd.h>

#include "mcts/mcts_search.h"
#include "core/move_generator.h"

MCTS_ThreadWorker::MCTS_ThreadWorker()
{

}

void MCTS_ThreadWorker::run_thread(const int & thread_id, const int &tree_entries, const std::string start_pos)
{

    std::cout << "initating thread [" << thread_id << "]" <<std::endl;
    std::string filepath = "training_data/thread_"+std::to_string(thread_id)+"_data.txt";
    int sleep_time_ms = 5000;

    ChessBoard root_cb(start_pos);
    std::unique_ptr<MoveGenerator>  move_gen  = std::make_unique<MoveGenerator> ();


    while(true)
    {
            std::unique_ptr<mcts::TreeSearch> tree_search = std::make_unique<mcts::TreeSearch> (move_gen.get());

            //init tree
            tree_search->init_tree(root_cb);

            //build tree
            tree_search->start_search(tree_entries);

            //if file exists just sleep
            while(file_exist(filepath))
            {
                usleep(sleep_time_ms);
            }

            //else write log info
            tree_search->log_data(filepath);

            //flush tree
            tree_search->clear_tree();
    }

}
