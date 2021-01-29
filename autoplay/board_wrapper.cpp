#include "board_wrapper.h"

#include "exception"

#include "utils/global_utils.cpp"

BoardWrapper::BoardWrapper(MoveGenerator* move_gen) :
    move_gen_(move_gen)
{

}

ChessMoveList BoardWrapper::get_actions()
{
    if(move_gen_)
    {
        movelist_ = move_gen_->get_legal_moves(cb_);
        has_queued_ = true;
    }

    return movelist_;
}

//status 0 = ongoing , 1 = white win, 2 = black win, 3 = draw

BoardInfo BoardWrapper::get_info() const
{
    BoardInfo binfo;

    int status = 0;

    int white_toact = cb_.is_white_toact();

    bool done = false;

    int rule_50 = cb_.get_rule50();
    int total_moves = cb_.get_total_moves(); // is this really incrememented when updating move?
    int hash_key = cb_.get_zobrist();

    float reward = 0.f;
    //todo, add threfold rep

    if(movelist_.empty())
    {
        done = true;

        if(move_gen_->king_under_attack(cb_))
        {
            if(white_toact)
            {
                reward =  -1.f;
                status = 2;
            }
            else
            {
                reward =  1.f;
                status = 1;
            }
        }
        else // stalemate my friend
        {
            reward = .5f;
            status = 4;

        }
    }
    else if(rule_50 >= 50 || total_moves > 150 || !cb_.has_mating_chance())
    {
        reward = .5f;
        status = 3;
        done = true;
    }

    binfo.status = status;
    binfo.rule_50 = rule_50;
    binfo.total_moves = total_moves;

    binfo.reward = reward;
    binfo.done = done;

    return binfo;
}

void BoardWrapper::step(ChessMove &move)
{
    cb_.update_from_move(move);

    cb_.mirror();

    movelist_.reset();

    has_queued_ = false;
}

void BoardWrapper::step(const int & move_idx)
{
    if( move_idx < 0 || move_idx >= movelist_.size())
    {
        print("index out of range! \n");
    }

    auto move = movelist_.get(move_idx);

    cb_.update_from_move(move);

    cb_.mirror();

    movelist_.reset();

    has_queued_ = false;
}

ChessBoard BoardWrapper::explore(const ChessMove &move)
{
    ChessBoard cb = cb_;

    cb.update_from_move(move);
    cb.mirror();

    return cb;
}

ChessBoard BoardWrapper::explore(const int &move_idx)
{
    ChessBoard cb = cb_;

    auto move = movelist_.get(move_idx);

    cb.update_from_move(move);
    cb.mirror();

    return cb;
}

void BoardWrapper::reset(const ChessBoard &cb)
{
    movelist_.reset();
    cb_ = cb;
    has_queued_ = false;

    starting_color_white_ = cb.get_whitetoact();
}

void BoardWrapper::show_state() const
{
    cb_.print_to_console();
}
