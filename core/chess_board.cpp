#include "chess_board.h"

#include <iostream>
#include <vector>
#include <cstring>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <sstream>
#include <string>

#include "utils/global_utils.cpp"

ChessBoard::ChessBoard() :
    rule50_(0),
    total_moves_(0),
    three_repeat_(0),
    white_toact_(1),
    enpassant_(-1)
{

}

ChessBoard::ChessBoard(std::string fen_string) :
    rule50_(0),
    total_moves_(0),
    three_repeat_(0),
    white_toact_(1),
    enpassant_(-1)
{
    set_from_fen(fen_string);
}

void ChessBoard::reset()
{
    pawns_ = 0x0;
    bishops_ = 0x0;
    knights_ = 0x0;
    rooks_ = 0x0;
    queens_ = 0x0;
    king_ = 0x0;

    enemy_pawns_ = 0x0;
    enemy_bishops_ = 0x0;
    enemy_knights_ = 0x0;
    enemy_rooks_ = 0x0;
    enemy_queens_ = 0x0;
    enemy_king_ = 0x0;

    our_pieces_  = 0x0;;
    enemy_pieces_  = 0x0;;
}

std::pair<char, uint64_t> ChessBoard::occupied_by(const short &s_idx) const
{
    const uint64_t s_idx_64 = 1ULL << s_idx;

    if (pawns_ & s_idx_64) return std::pair<char, uint64_t> ('P', pawns_);
    else if (enemy_pawns_ & s_idx_64) return std::pair<char, uint64_t> ('p', enemy_pawns_);

    else if (knights_ & s_idx_64) return std::pair<char, uint64_t> ('N', knights_);
    else if (bishops_ & s_idx_64) return std::pair<char, uint64_t> ('B', bishops_);
    else if (rooks_ & s_idx_64) return std::pair<char, uint64_t> ('R', rooks_);
    else if (queens_ & s_idx_64) return std::pair<char, uint64_t> ('Q', queens_);
    else if (king_ & s_idx_64 ) return std::pair<char, uint64_t> ('K', king_);

    else if (enemy_knights_ & s_idx_64) return std::pair<char, uint64_t> ('n', enemy_knights_);
    else if (enemy_bishops_ & s_idx_64) return std::pair<char, uint64_t> ('b', enemy_bishops_);
    else if (enemy_rooks_ & s_idx_64) return std::pair<char, uint64_t> ('r', enemy_rooks_);
    else if (enemy_queens_ & s_idx_64) return std::pair<char, uint64_t> ('q', enemy_queens_);
    else if (enemy_king_ & s_idx_64) return std::pair<char, uint64_t> ('k', enemy_king_);

    else return std::pair<char, uint64_t> (',', 0);
}

void ChessBoard::set_from_fen(std::string fen_string)
{
        int row = 7;
        int col = 0;

        // Remove any trailing whitespaces to detect eof after the last field.
        fen_string.erase(std::find_if( fen_string.rbegin(), fen_string.rend(), [](char c) { return !std::isspace(c); } ).base(),
                         fen_string.end());

        std::istringstream fen_str(fen_string);
        std::string board;

        fen_str >> board;
        std::string who_to_move = "w";
        if (!fen_str.eof()) fen_str >> who_to_move;
        std::string castlings = "-";
        if (!fen_str.eof()) fen_str >> castlings;
        std::string en_passant = "-";
        if (!fen_str.eof()) fen_str >> en_passant;
        int rule50 = 0;
        if (!fen_str.eof()) fen_str >> rule50;
        int total_moves = 1;
        if (!fen_str.eof()) fen_str >> total_moves;

        for (char c : board) {

          if (c == '/') {
            --row;
            col = 0;
            continue;
          }
          if (std::isdigit(c)) {
            col += c - '0';
            continue;
          }

          uint64_t idx =  row_col_to_idx(row,col);


          if (std::isupper(c)) {
            // White piece.
            our_pieces_ |= 1ULL << idx;
          } else {
            // Black piece.
            enemy_pieces_|= 1ULL << idx;
          }

        if (c == 'P') pawns_ |=  1ULL << idx;
        else if (c == 'N') knights_ |= 1ULL << idx;
        else if (c == 'B') bishops_ |=1ULL << idx;
        else if (c == 'R') rooks_ |= 1ULL << idx;
        else if (c == 'Q') queens_ |= 1ULL << idx;
        else if (c == 'K')   king_|= 1ULL << idx;
        else if (c == 'p') enemy_pawns_ |=1ULL << idx;
        else if (c == 'n') enemy_knights_ |=1ULL << idx;
        else if (c == 'b') enemy_bishops_ |=1ULL << idx;
        else if (c == 'r') enemy_rooks_ |= 1ULL << idx;
        else if (c == 'q') enemy_queens_ |=1ULL << idx;
        else if (c == 'k') enemy_king_ |= 1ULL << idx;

          ++col;
        }        

        rule50_ = rule50;
        total_moves_ = total_moves;

        if(en_passant == "-") enpassant_ = -1 ;
        else enpassant_ = std::stoi(en_passant);

        castling_.setWe_00(castlings.find('K') != std::string::npos);
        castling_.setWe_000(castlings.find('Q') != std::string::npos);
        castling_.setEnemy_00(castlings.find('k') != std::string::npos);
        castling_.setEnemy_000(castlings.find('q') != std::string::npos);
}

std::vector<char> ChessBoard::_fill_printer(const int &print_mode, char spec_type) const
{
    std::vector<char> printer;

    std::vector<char> notations;
    if( print_mode == 0 )  notations =  std::vector<char> {'P', 'N', 'B', 'R', 'Q', 'K', 'p', 'n', 'b', 'r', 'q', 'k'};
    else if( print_mode == 1)  notations = std::vector<char> {'P', 'N', 'B', 'R', 'Q', 'K'};
    else if (print_mode == 2) notations =  std::vector<char> {'p', 'n', 'b', 'r', 'q', 'k'};
    else if (print_mode == 3) notations =  std::vector<char> {spec_type};

    for (int i = 0 ; i<64 ; ++i)
    {
        auto piece_pair = occupied_by(i);

        if(contains(piece_pair.first , notations))
            printer.emplace_back(piece_pair.first);

        else
             printer.emplace_back(',');
    }

    return printer;
}

void ChessBoard::print_to_console(const int& print_mode, char spec_type ) const
{
    auto printer = _fill_printer(print_mode, spec_type);
    int idx = 56;

    print("___A__B__C__D__E__F__G__H__");

    std::cout << int(idx/8) + 1;

    while( idx >= 0)
    {
        if ((idx + 1) % 8 == 0)
        {
            std::cout << " |" << printer.at(idx) << " |" << std::endl;
            idx -= 15;
            if (int(idx / 8) + 1 > 0)
            {
                std::cout << (int(idx / 8) + 1);
            }
        }
        else
        {
            std::cout << " |" << printer.at(idx);
            idx += 1;
        }
    }   
    print("___A__B__C__D__E__F__G__H__");
    print("\n");
}

void ChessBoard::print_bitboard(const uint64_t &bitboard)
{
    ChessBoard cb;
    cb.pawns_ = bitboard;

    cb.print_to_console('P');
}

