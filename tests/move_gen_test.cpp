#include "tests/move_gen_test.h"
#include "autoplay/board_wrapper.h"

void run_move_gen_test()
{
    auto fen_pos = "rnbqkbnr/pppppppp/8/8/1K6/8/1PPPPPPP/RNBQ1BNR w kq - 0 1";
    ChessBoard cb(fen_pos);

    MoveGenerator move_gen;
    auto moves = move_gen.get_legal_moves(cb);

//    Timer t0;
//    int iters = 100000;

//    for(int i = 0 ; i < iters ; ++i)
//    {
//
//    }

//    auto elapsed_time = t0.elapsed();

//    std::cout << "moves per second : " << 20 * iters / elapsed_time;
    //print(t0.elapsed());
    moves.first.print_moves();
}

void apply_move_and_count(MoveGenerator &move_gen, int & total_moves, ChessBoard cb, int & depth, const int& max_depth, Timer& t, float & _time)
{

    if(depth == max_depth) return;

    int d = depth + 1;

    print(d);

    auto moves = move_gen.get_legal_moves(cb).first;

    auto n_moves = moves.size();
    total_moves += n_moves;

    cb.print_to_console();
    moves.print_moves();

    if( d == max_depth) return;

    if(n_moves == 0) return;
    else
    {
        for(int i = 0; i < n_moves; ++i)
        {
             auto t0 = t.elapsed();
             cb.update_from_move(moves.get(i));
             cb.mirror();
             auto t1 = t.elapsed();
             _time += t1-t0;
             apply_move_and_count(move_gen, total_moves, cb, d, max_depth, t, _time);
        }
    }
}

void move_gen_test_ply(const int & max_depth)
{
    MoveGenerator move_gen;

    auto fen_pos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    BoardWrapper env(&move_gen);

    ChessBoard cb(fen_pos);


    Timer t0;
    float _time = 0.f;
    int depth = 0;

    std::vector<ChessBoard> cbs;

    cbs.emplace_back(cb);

    while(depth != max_depth)
    {
        std::vector<ChessBoard> ncbs;
        int total_moves = 0;

        for(const auto & cb : cbs)
        {
            env.reset(cb);
            auto moves = env.get_actions();
            auto n_moves = moves.size();
            total_moves += n_moves;

            for(int i = 0; i < n_moves; ++i)
            {
                auto ncb = env.explore(moves.get(i));
                ncbs.emplace_back(ncb);
            }
        }

        cbs.clear();
        cbs = ncbs;

        std::cout << "Ply : " << depth << " moves [" << total_moves << "]" << std::endl;

        ++depth;
    }



    auto time = t0.elapsed();

    std::cout << "calculation took : " << time << " seconds " << std::endl;

    std::cout << " mirror and update took" << _time << " seconds";

}
