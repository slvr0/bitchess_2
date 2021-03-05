#pragma once

#include <iostream>
#include <vector>

#include "core/chess_castle.h"
#include "core/chess_move.h"

class ChessBoard
{
public:
    ChessBoard();
    ChessBoard(std::string fen_string);

    void set_from_fen(std::string fen_string);

    inline uint64_t get_pawns() const { return pawns_;}
    inline uint64_t  get_knights() const { return knights_;}
    inline uint64_t  get_bishops() const { return bishops_;}
    inline uint64_t get_rooks() const { return rooks_;}
    inline uint64_t get_queens() const { return queens_;}
    inline uint64_t get_king() const { return king_;}
    inline uint64_t get_enemy_pawns() const { return enemy_pawns_;}
    inline uint64_t get_enemy_knights() const { return enemy_knights_;}
    inline uint64_t get_enemy_bishops() const { return enemy_bishops_;}
    inline uint64_t get_enemy_rooks() const { return enemy_rooks_;}
    inline uint64_t get_enemy_queens() const { return enemy_queens_;}
    inline uint64_t get_enemy_king() const { return enemy_king_;}

    inline uint64_t get_our_pieces() const {return our_pieces_;}
    inline uint64_t get_enemy_pieces() const {return enemy_pieces_;}
    inline uint64_t get_occ() const {return our_pieces_ | enemy_pieces_;}

    inline uint get_rule50() const { return rule50_;}
    inline uint get_total_moves() const {return total_moves_;}
    inline int is_white_toact() const { return white_toact_;}

    inline int get_threefold() const {return three_repeat_;}
    inline int get_whitetoact() const {return white_toact_;}
    inline int get_enpassant() const {return enpassant_;}

    void print_to_console(const int & print_mode= 0, char spec_type= ' ') const;
    void print_bitboard(const uint64_t & bitboard) const;
    void reset();
    void mirror();

    void add_piece(const uint8_t & square, char ptype);
    void remove_piece(const uint8_t & square);

    std::pair<char, uint64_t> occupied_by(const short & s_idx) const ;
    std::pair<int, uint64_t> occupied_by_res_int(const short & s_idx) const ;

    Castling castling_;

    bool has_mating_chance() const;

    void update_from_move(const ChessMove& move);

    void set_zobrist();
    void update_zobrist(const ChessMove& move);
    inline uint64_t get_zobrist() const { return z_hash_;}

    std::string fen() const ;

private :
    std::string fen_string_ = "";

    uint rule50_ = 0;
    uint total_moves_ = 0;

    int three_repeat_ = 0;
    int white_toact_ = 1;
    int enpassant_ = -1;

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

    uint64_t our_pieces_{0x0};
    uint64_t enemy_pieces_{0x0};

    std::vector<char> _fill_printer(const int & print_mode, char spec_type) const;

    uint64_t z_hash_{0x0};
};


