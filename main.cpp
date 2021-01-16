
#include <iostream>
#include "utils/global_utils.cpp"

using namespace std;

#include "tests/chess_board_tests.cpp"
#include "tests/attack_tables_test.cpp"
#include "tests/move_gen_test.cpp"

    int main()
{
        ChessAttackTables attack_tables ;
        attack_tables.init_tables(); //tables are ureached globally , doesnt require instance of this( easier for multithreading lookups later)


        //run_chessboard_test();
        //run_attack_tables_test();
        run_move_gen_test();








}
