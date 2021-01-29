#include <iostream>

#include <thread>
#include <pthread.h>

#include "tests/chess_board_tests.cpp"
#include "utils/global_utils.cpp"
#include "core/chess_board.h"
#include "core/zobr_hash.h"
#include "autoplay/board_wrapper.h"

#include "mcts/mcts_node.h"
#include "mcts/mcts_rollout.h"
#include "mcts/mcts_search.h"

#include "net/data_encoder.h"

#include "core/chess_attack_tables.h"

#include "core/chess_attack_tables.h"
#include "autoplay/auto_mcts_search_thread.h"

int main()
{
    init_tables();
    init_prehash();

    auto fen0 = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";


    int max_entries = 10000;
    int n_threads = 5;

    std::vector<std::thread> vth;

    std::string interesting_start_positions[10] =
    {
        "rnbqk2r/pp1p1pbp/2p2np1/4p3/2P5/2N1P1P1/PP1P1PBP/R1BQK1NR w KQkq - 0 6", //english std
        "rr1q3k/2bnn2p/3pp3/5pp1/2PPP3/2BNP1P1/1P1QRPBP/1R4K1 w q - 0 1",
        "8/8/3R4/5PP1/4NN2/5K2/8/r4k2 w - - 0 1",
        "4k2b/2b2q2/1nn1p3/4P3/6p1/P2PN2p/2Q1N1PP/R2K2P1 w Q - 0 1",
        "rnbqk1nr/ppp2ppp/4p3/3p4/1b1PP3/2N5/PPP2PPP/R1BQKBNR w KQkq - 2 4", //french winaware
        "r1bqkbnr/2pp1ppp/p1n5/1p2p3/B3P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 5", //ruy lopz
        "r1bqk2r/pp1n1ppp/2p1pn2/8/1bpP4/2N2NP1/PP2PPBP/R1BQ1RK1 w kq - 0 8" //catalan/slav exchange
        "rnbqk2r/ppp1ppbp/3p1np1/8/2PPP3/2N5/PP3PPP/R1BQKBNR w KQkq - 0 5" //kings indian
        "rnbqkbnr/pppp1ppp/8/4P3/8/8/PPP1PPPP/RNBQKBNR b KQkq - 0 2" //englund gambit
        "r1bqk2r/ppp2ppp/2p2n2/2b5/4P3/2N5/PPPP1PPP/R1BQKB1R w KQkq - 2 6",//stafford
    };

    for(int i = 0 ; i < n_threads; ++i)
    {
        vth.emplace_back(std::thread(std::thread(MCTS_ThreadWorker::run_thread, i, max_entries, interesting_start_positions[i])));
    }

    for(auto & thread : vth) thread.join();
    vth.clear();

//    for(auto & thread : vth) thread.join();

    //MCTS_ThreadWorker::run_thread(0, max_entries, fen0);





//    auto fen_win = "rnb1kbnr/pppp1ppp/8/4p3/3P4/P2Q2q1/1PP1PPPP/RNB1KBNR w KQkq - 1 4";
//    auto f3 = "4k3/P7/8/8/8/4K3/8/R7 w - - 0 1";
//    auto win_ffs = "4k3/P7/8/8/2QQ4/2QQK3/8/R7 w - - 0 1";
//    auto test = "8/8/8/1k6/5b2/8/rp1N4/2KRq3 w - - 0 1";

//    ChessBoard cb(fen_win);

//    std::unique_ptr<MoveGenerator>  move_gen  = std::make_unique<MoveGenerator> ();

//    mcts::TreeSearch mc_tree_search(move_gen.get());

//    mc_tree_search.init_tree(cb);

//    int max_entries = 50;
//    mc_tree_search.start_search(max_entries);



}
