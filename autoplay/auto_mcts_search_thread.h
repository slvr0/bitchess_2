#pragma once

#include <iostream>

#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <fstream>


inline bool file_exist (const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}


class MCTS_ThreadWorker
{
public:
    MCTS_ThreadWorker();
    MCTS_ThreadWorker(MCTS_ThreadWorker &&) = default;

    static void run_thread(const int & thread_id, const int &tree_entries, const int & tree_rollouts, const std::string start_pos);


};
