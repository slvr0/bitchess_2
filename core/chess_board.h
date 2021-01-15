#pragma once

#include <iostream>
#include <vector>


#include "core/chess_castle.h"

class ChessBoard
{
public:

    ChessBoard();
    ChessBoard(std::string fen_string);

    void set_from_fen(std::string fen_string);

    inline uint64_t get_pawns() { return pawns_;}
    inline uint64_t  get_knights() { return knights_;}
    inline uint64_t  get_bishops() { return bishops_;}
    inline uint64_t get_rooks() { return rooks_;}
    inline uint64_t get_queens() { return queens_;}
    inline uint64_t get_king() { return king_;}
    inline uint64_t get_enemy_pawns() { return enemy_pawns_;}
    inline uint64_t get_enemy_knights() { return enemy_knights_;}
    inline uint64_t get_enemy_bishops() { return enemy_bishops_;}
    inline uint64_t get_enemy_rooks() { return enemy_rooks_;}
    inline uint64_t get_enemy_queens() { return enemy_queens_;}
    inline uint64_t get_enemy_king() { return enemy_king_;}

    void print_to_console(const int & print_mode= 0, char spec_type= ' ');
    void print_bitboard(const uint64_t & bitboard);
    void reset();

    std::pair<char, uint64_t> occupied_by(const short & s_idx) const ;

private :

    Castling castling_;

    std::string fen_string_;

    uint rule50_;
    uint total_moves_;
    int three_repeat_;
    int white_toact_;
    int enpassant_;

    uint64_t pawns_{0x0};
    uint64_t bishops_{0x0};
    uint64_t knights_{0x0};
    uint64_t rooks_{0x0};
    uint64_t queens_{0x0};
    uint64_t king_{0x0};

    uint64_t enemy_pawns_{0x0};
    uint64_t enemy_bishops_{0x0};
    uint64_t enemy_knights_{0x0};
    uint64_t enemy_rooks_{0x0};
    uint64_t enemy_queens_{0x0};
    uint64_t enemy_king_{0x0};

    uint64_t our_pieces_;
    uint64_t enemy_pieces_;

    std::vector<char> _fill_printer(const int & print_mode, char spec_type);

};
