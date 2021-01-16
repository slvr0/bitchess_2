#include "chess_move.h"

#include "utils/global_utils.cpp"

ChessMove::ChessMove() :
    from_(-1),
    to_(-1),
    ptype_('P'),
    spec_action_(""),
    promotion_("")
{

}

ChessMove::ChessMove(int from, int to, char ptype, std::string spec_action, std::string promotion) :
    from_(from),
    to_(to),
    ptype_(ptype),
    spec_action_(spec_action),
    promotion_(promotion)
{

}

int ChessMove::from() const
{
    return from_;
}

void ChessMove::setFrom(int from)
{
    from_ = from;
}

void ChessMove::setTo(const int & to)
{
    to_ = to;
}

int ChessMove::to() const
{
    return to_;
}

char ChessMove::ptype() const
{
    return ptype_;
}

void ChessMove::setPtype(char ptype)
{
    ptype_ = ptype;
}

std::string ChessMove::spec_action() const
{
    return spec_action_;
}

void ChessMove::setSpec_action(const std::string &spec_action)
{
    spec_action_ = spec_action;
}

std::string ChessMove::promotion() const
{
    return promotion_;
}

void ChessMove::setPromotion(const std::string &promotion)
{
    promotion_ = promotion;
}

ChessMoveList::ChessMoveList()
{

}

void ChessMoveList::add_move(ChessMove move)
{
    if(move.spec_action() == "=" && move.promotion() == "") //a little haxx, but its to prevent it from being reinserted and spawning 4 additional moves
    {
        moves.emplace_back(ChessMove(move.from_, move.to_, move.ptype_, "=", "N" ));
        moves.emplace_back(ChessMove(move.from_, move.to_, move.ptype_, "=", "B" ));
        moves.emplace_back(ChessMove(move.from_, move.to_, move.ptype_, "=", "R" ));
        moves.emplace_back(ChessMove(move.from_, move.to_, move.ptype_, "=", "Q" ));
    }
    else
    {
        moves.emplace_back(move);
    }
}

void ChessMoveList::print_moves()
{
    print("printing moves...");

    for(const auto & move : moves) std::cout << move.ptype_ << " from: " << move.from_ << " to: " << move.to_ << " spec_action :  " << move.spec_action_ << " promo: " << move.promotion_ << std::endl;
}

std::vector<ChessMove> ChessMoveList::get_moves() const
{
    return moves;
}
