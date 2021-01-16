#pragma once

#include <iostream>
#include <vector>

class ChessMove
{
public:
    friend class ChessMoveList;

    ChessMove();
    ChessMove(int from, int to, char ptype, std::string spec_action ="", std::string promotion="");

    int from() const;
    void setFrom(int from);

    char ptype() const;
    void setPtype(char ptype);

    std::string spec_action() const;
    void setSpec_action(const std::string &spec_action);

    std::string promotion() const;
    void setPromotion(const std::string &promotion);

    void setTo(const int & to);
    int to() const;


private:
    int from_{0};
    int to_{0};
    char ptype_ = ' ';
    std::string spec_action_ = "";
    std::string promotion_ = "";
};

class ChessMoveList
{
public:
    ChessMoveList();

    void add_move(ChessMove move);

    void print_moves();

    std::vector<ChessMove> get_moves() const;

private :
    std::vector<ChessMove> moves;
};
