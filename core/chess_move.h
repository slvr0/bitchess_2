#pragma once

#include <iostream>
#include <vector>

#include "utils/chess_enums.cpp"

class ChessMove
{
public:
    friend class ChessMoveList;

    ChessMove();
    ChessMove(int from, int to, char ptype, std::string spec_action ="", char promotion= ' ');

    int from() const;
    void setFrom(int from);

    char ptype() const;
    void setPtype(char ptype);

    std::string spec_action() const;
    void setSpec_action(const std::string &spec_action);

    char promotion() const;
    void setPromotion(const char &promotion);

    void setTo(const int & to);
    int to() const;

    inline void print_move() const
    {
        std::cout << ptype_ << " from: " << board_notations[from_] << " to: " << board_notations[to_] << " spec_action :  " << spec_action_ << " promo: " << promotion_ << std::endl;
    }

private:
    int from_{0};
    int to_{0};
    char ptype_ = ' ';
    std::string spec_action_ = "";
    char promotion_ = ' ';
};

class ChessMoveList
{
public:
    ChessMoveList();

    void add_move(ChessMove move, bool white_toact=true);

    void print_moves();

    inline size_t size() const
    {
        return moves.size();
    }

    inline ChessMove get(const int & idx) const { return moves.at(idx);}

    std::vector<ChessMove> get_moves() const;

    inline void reset() {moves.clear();}
    inline bool empty() const { return moves.empty();}



private :
    std::vector<ChessMove> moves;
};
