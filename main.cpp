#include <iostream>
#include <memory>

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

#include "tests/move_gen_test.h"
#include "mcts/mcts_node.h"

#include "tests/rollout_test.h"

#include "mcts/mcts_signal_initiater.h"
#include "utils/chess_enums.cpp"

#include <QCoreApplication>
#include <cassert>

#include "mcts/cached_positions.h"

#include "net/mqtt_client.h"

int main(int argc, char** argv)
{
//    {
//        //sets up mcts environment with mqtt signals in a qapp
//        init_tables();
//        init_prehash();

//        qRegisterMetaType<QMQTT::Message>("QMQTT::Message");

//        QCoreApplication app(argc, argv);

//        MQTT_PIPE_THREAD comm_thread;

//        auto host = QHostAddress::LocalHost;
//        auto port = 1883;

//        {
//            MQTT_PIPE* mqtt_receiver0 = new MQTT_PIPE(host, port);

//            mqtt_receiver0->set_topic(QString("mcts_tree_init"));
//            mqtt_receiver0->set_publish_topic(QString("mcts_tree_finish"));

//            mqtt_receiver0->connectToHost();
//            comm_thread.append_and_allocate_client(mqtt_receiver0);
//        }

//        {
//            MQTT_PIPE* mqtt_receiver1 = new MQTT_PIPE(host, port);
//            mqtt_receiver1->set_topic(QString("mcts_cache_position"));
//            mqtt_receiver1->set_publish_topic(QString("mcts_nn_que"));

//            mqtt_receiver1->connectToHost();
//            comm_thread.append_and_allocate_client(mqtt_receiver1);
//        }

//        MCTSSignalInitiater mcts_signal_initiater(comm_thread);

//        comm_thread.start();

//        return app.exec();
//    }

//    init_tables();
//    init_prehash();

//    MoveGenerator movegen;

//    auto pos  = "rn1qk2r/pppbnp1p/4p3/3P2p1/Pb4P1/1PN4P/2P1KpB1/R1BQ2NR b qk - 0 17";
//    ChessBoard cb(pos);
//    print(cb.fen());
//    auto moves = movegen.get_legal_moves(cb).first;
//    moves.print_moves();

    //test output, take a chess position, print all NN idcs on moves
//    {
//        init_tables();
//        init_prehash();
//        auto pos  = "rnbqk2r/p1p2ppp/1p2p2n/3P4/1b1P4/2N5/PPP2PPP/R1BQKBNR b qk - 0 9";
//        ChessBoard cb(pos);

//        MoveGenerator move_gen_;
//        auto moves = move_gen_.get_legal_moves(cb).first;

//        DataEncoder encoder_;

//        for(const auto & move : moves.get_moves())
//        {
//            auto nn_idx = encoder_.move_as_nn_input(move);
//            std::cout << nn_idx << ",";
//        }
//        print("");
//    }

    // "rnbqk1nr/ppp2ppp/4p3/3p4/1b1PP3/2N5/PPP2PPP/R1BQKBNR KQkq - 2 4" == "rnbkqb1r/ppp2ppp/5n2/3pp1B1/4P3/3P4/PPP2PPP/RN1KQBNR w - - 4 1"



//    {
//        init_tables();
//        init_prehash();

//        ChessBoard cb("2rq2k1/pprbbpp1/4pn1p/3pN3/3BP3/1N6/P1n1BPPP/RQ3RK1 b - - 0 21");

//        MoveGenerator move_gen;

//        auto moves = move_gen.get_legal_moves(cb).first;

//        DataEncoder nn_encoder;

//        moves.print_moves();

//        for(const auto & move :  moves.get_moves())
//        {
//            auto nn_idx = nn_encoder.move_as_nn_input(move, cb.get_whitetoact());
//            std::cout << nn_idx << std::endl;
//        }
//    }




    init_tables();
    init_prehash();

     move_gen_test_ply(4);
    //test

//    auto fen0 = "rnbqk1nr/ppp2ppp/4p3/3p4/1b1PP3/2N5/PPP2PPP/R1BQKBNR w KQkq - 2 4";

//    ChessBoard root_cb(fen0);
//    std::unique_ptr<MoveGenerator>  move_gen  = std::make_unique<MoveGenerator> ();


//    auto state_move = move_gen->get_legal_moves(root_cb);

//    auto moves =  state_move.first;

//    moves.print_moves();

    //make simulation env on thread

//    auto fen0 = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1";

//    int max_entries = 10000000;
//    int n_rollouts = 1500000;

//    int n_threads = 1;

//    std::vector<std::thread> vth;

//    std::vector<std::string> interesting_start_positions
//    {
//        "1kq5/2q5/8/8/8/8/4QQ2/5K2 b - - 0 1", //english std, from black queen hangs
//        "rnbqk2r/pp1p1pbp/2p2np1/4p3/2P5/2N1P1P1/PP1P1PBP/R1BQK1NR w KQkq - 0 6", //english std
//        "rr1q3k/2bnn2p/3pp3/5pp1/2PPP3/2BNP1P1/1P1QRPBP/1R4K1 w q - 0 1",
//        "8/8/3R4/5PP1/4NN2/5K2/8/r4k2 w - - 0 1",
//        "4k2b/2b2q2/1nn1p3/4P3/6p1/P2PN2p/2Q1N1PP/R2K2P1 w Q - 0 1",
//        "r1bqkbnr/2pp1ppp/p1n5/1p2p3/B3P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 5", //ruy lopz
//        "r1bqk2r/pp1n1ppp/2p1pn2/8/1bpP4/2N2NP1/PP2PPBP/R1BQ1RK1 w kq - 0 8" //catalan/slav exchange
//        "rnbqk2r/ppp1ppbp/3p1np1/8/2PPP3/2N5/PP3PPP/R1BQKBNR w KQkq - 0 5" //kings indian
//        "rnbqkbnr/pppp1ppp/8/4P3/8/8/PPP1PPPP/RNBQKBNR b KQkq - 0 2" //englund gambit
//        "r1bqk2r/ppp2ppp/2p2n2/2b5/4P3/2N5/PPPP1PPP/R1BQKB1R w KQkq - 2 6",//stafford
//    };

////get 20 sharp mid game positions , bot should learn to capture
//    std::vector<std::string> mid_pos
//    {
//        "8/1b3kp1/4Np1p/pp6/2nPPN2/1r3PP1/7P/2R3K1 w - - 0 32",
//        "4kb1r/1p3ppp/p2Np1b1/3pP3/P2P4/4PN2/1P4PP/2R3K1 b k - 1 18",
//        "r3r1k1/ppqn1ppp/2pb4/5p1b/3P4/1QP1B2P/PP1NBPP1/R3R1K1 b - - 3 14",
//        "2r1r1k1/pp2bppp/1q2pnB1/3p2B1/3n4/P1N2N1P/1PPQ1PP1/1R2R1K1 b - - 0 16",
//        "2r3k1/5pp1/2N1pn1p/p2p4/2BP4/B3P3/1P1n1PPP/2R3K1 w - - 1 27",
//        "r3kbnr/pp3ppp/1qn1p3/3pP3/P2P2b1/1P2BN2/5PPP/RN1QKB1R b KQkq - 0 9",
//        "rn2k2r/pp1qbppp/2p2p2/5b2/3P4/3B1N2/PPP2PPP/R1BQR1K1 b kq - 7 9",
//        "r2r2k1/pp3ppp/4np2/2pq4/3P3N/1PP3P1/P1Q1RPP1/4R1K1 w - - 0 20",
//        "1r3rk1/3bb1pp/1q1ppn2/2p1n3/1p3N2/1PN1P1P1/PB2QPBP/R2R2K1 w - - 0 17",
//        "rn1q1rk1/p3bppp/bp1ppn2/8/2PP1B2/2N2N2/PP2BPPP/R2Q1RK1 w - - 2 10",
//        "r4rk1/4bp1p/p7/2n5/6B1/4Q3/qP3PPP/2RR2K1 w - - 0 25",
//        "rn1q1rk1/1b2bpp1/3ppn1p/p7/1p1NP1PP/4BP2/PPPQN3/1K1R1B1R b - - 0 14",
//        "2rr2k1/pp3pp1/2q2n1p/3p4/3R4/1QN1P2P/PP3PP1/3R2K1 w - - 5 23",
//        "r3kb1r/ppp1pppb/1qn2n1p/7P/2B2PP1/2NP4/PPPBQ3/R3K1NR b KQkq - 0 12",
//        "b2r2k1/4qppp/N7/1B6/3n1p2/5P2/PPP3PP/4QRK1 b - - 0 22",
//        "rnbqkb1r/ppp2ppp/4p3/3p4/2PP2n1/2N1P3/PP3PPP/R1BQKBNR w KQkq - 1 5",
//        "r1bqkb1r/ppp1pppp/2n2n2/1N1p4/3P1B2/8/PPP1PPPP/R2QKBNR b KQkq - 5 4",
//        "rnb1kb1r/ppp1pppp/3q1n2/3p4/3P1B2/2N3P1/PPP1PP1P/R2QKBNR b KQkq - 2 4",
//        "5k2/2q2ppp/8/2n5/1P3b2/8/4NPPP/2R3K1 w - - 0 1",
//        "r1b2rk1/p1p1qppp/1pn1p3/3p4/1b1P1Nn1/2N1P1P1/PPPB1PBP/R2Q1RK1 w - - 9 10"
//    };

////    for(int i = 0 ; i < n_threads; ++i)
////    {
////        vth.emplace_back(std::thread(MCTS_ThreadWorker::run_thread, i, max_entries, n_rollouts, "rnbqk1nr/ppp2ppp/4p3/3p4/1b1PP3/2N5/PPP2PPP/R1BQKBNR w KQkq - 2 4"));
////    }

//    int current_folder = 1;

//    for(auto & pos : mid_pos)
//    {
//        MCTS_ThreadWorker threadworker;
//        threadworker.run_thread(0, max_entries, n_rollouts, pos, current_folder);
//        current_folder++;
//    }
//    for(auto & thread : vth) thread.join();
//    vth.clear();





 }
