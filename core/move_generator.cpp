#include "move_generator.h"

#include "utils/global_utils.cpp"
#include "core/chess_move.h"

#include "chess_attack_tables.h"

MoveGenerator::MoveGenerator()
{

}

 uint64_t MoveGenerator::get_kingmoves(unsigned long idx)
 {
     return get_king_moves(idx);
 }

std::pair<ChessMoveList, int> MoveGenerator::get_legal_moves(const ChessBoard &cb)
{
    ChessMoveList movelist;
    PushPinInfo pp_info;

    bool found_king = false;
    unsigned long king_idx = -1;

    uint64_t our_pieces = cb.get_our_pieces();
    uint64_t enemy_pieces  = cb.get_enemy_pieces();
    uint64_t occ = cb.get_occ();

    //these data points doesnt fit in the push pin calculation algoritm and are added separate when scanning the board
    uint64_t knight_capture_mask {0x0};
    unsigned long non_slider_attackers = 0;

    uint64_t attack_mask {0x0};
    uint64_t attack_mask_noking{0x0};

    uint64_t our_king_moves{0x0}; //this can also just be indexed, havent found a table for this yet

     //could do bit iter over occ here instead. try it later (wasnt faster, bit iter is kinda slow)

    const uint64_t king = cb.get_king();

    for(int idx = 0 ; idx < 64 ; ++idx)
    {
        if (1ULL << idx & cb.get_king())
        {            
            our_king_moves = get_kingmoves(idx);

            found_king = true;
            king_idx = idx;

            pp_info = get_push_pin_info(idx, our_pieces, enemy_pieces, cb.get_enemy_bishops(), cb.get_enemy_rooks(), cb.get_enemy_queens());
        }
        else if(1ULL << idx & cb.get_pawns()) append_pseudolegal_pawnmoves(cb, idx, movelist);
        else if(1ULL << idx & cb.get_knights()) append_pseudolegal_slidermoves(cb, idx, movelist, 'N');
        else if(1ULL << idx & cb.get_bishops()) append_pseudolegal_slidermoves(cb, idx, movelist, 'B');
        else if(1ULL << idx & cb.get_rooks()) append_pseudolegal_slidermoves(cb, idx, movelist, 'R');
        else if(1ULL << idx & cb.get_queens()) append_pseudolegal_slidermoves(cb, idx, movelist, 'Q');

        else if( 1ULL << idx & cb.get_enemy_pawns())
        {
            uint64_t attacks = get_pawn_attacks_rev(idx);
            if (attacks & cb.get_king())
            {
                non_slider_attackers++;
            }
            attack_mask |= attacks;
            attack_mask_noking |= attacks;
        }
        else if( 1ULL << idx & cb.get_enemy_knights())
        {
          uint64_t attacks = get_knight_attacks(idx);
          attack_mask |= attacks;
          attack_mask_noking |= attacks;

          if (attacks & cb.get_king())
          {
              knight_capture_mask |= 1ULL << idx;
              non_slider_attackers++;
          }
        }
        else if( (1ULL << idx) & cb.get_enemy_bishops())
        {
            uint64_t attacks = get_bishop_attacks(occ, idx);
            uint64_t attacks_noking = get_bishop_attacks(occ & ~king, idx);
            attack_mask |= attacks;
            attack_mask_noking |= attacks_noking;
        }
        else if( (1ULL << idx) & cb.get_enemy_rooks())
        {
            uint64_t attacks = get_rook_attacks(occ, idx);
            uint64_t attacks_noking = get_rook_attacks(occ & ~king, idx);

            attack_mask |= attacks;
            attack_mask_noking |= attacks_noking;
        }
        else if( (1ULL << idx )& cb.get_enemy_queens())
        {
          uint64_t attacks = get_bishop_attacks(occ, idx) | get_rook_attacks(occ, idx);

          uint64_t attacks_noking = get_bishop_attacks(occ & ~king, idx) | get_rook_attacks(occ & ~king, idx);

          attack_mask |= attacks;
          attack_mask_noking |= attacks_noking;
        }
        else if( (1ULL << idx) & cb.get_enemy_king())
        {
            uint64_t  attacks = get_king_moves(idx);
            attack_mask |= attacks;
            attack_mask_noking |= attacks;
        }
    }

    pp_info.n_attackers += non_slider_attackers;
    pp_info.capture_mask |= knight_capture_mask;

    our_king_moves &= ~our_pieces;
    our_king_moves &= ~attack_mask_noking;

    our_king_moves &= ~cb.get_enemy_king();

    //finally remove movement to protected enemy pieces
    our_king_moves &= ~(attack_mask_noking & enemy_pieces);

   ChessMoveList legal_movelist;

   int idx_count = pop_count(our_king_moves);

   for(int i = 0; i< idx_count; ++i)
   {
       unsigned long lb = least_bit(our_king_moves);

       //do logic with lb right away;
       legal_movelist.add_move(ChessMove(king_idx, lb, 'K'));

       if(our_king_moves == 0) break;

       our_king_moves ^= 1ULL << lb;
   }

   if(pp_info.n_attackers > 1) return std::pair<ChessMoveList, int> ( legal_movelist, pp_info.n_attackers );

   //now check which moves are legal or not based on the Pin Push Capture information collected

   for(const auto & move : movelist.get_moves())
   {
       int from = move.from();
       int to = move.to();
       std::string spec_action = move.spec_action();

       bool is_pinned = false;
       int pinned_index = -1;

       int p_index = 0;
       for(const auto & pin_pair : pp_info.pins)
       {
            if(from==pin_pair.first)
            {
                is_pinned = true;
                pinned_index = p_index;
                break;
            }
            p_index++;
       }

       if(pp_info.n_attackers == 1 && 1ULL << to &  (pp_info.push_mask_full | pp_info.capture_mask) && !is_pinned )
       {
           legal_movelist.add_move(move);
           continue;
       }

       //we're pinned, but the move is to attack the pinner. is all good
       else if(is_pinned && 1ULL << to & pp_info.pins.at(pinned_index).second)
       {
             legal_movelist.add_move(move);
             continue;
       }
       else if (!is_pinned && pp_info.n_attackers == 0 ) legal_movelist.add_move(move);
   }

   //last, castle moves
   if (pp_info.n_attackers == 0)
   {
     uint64_t w_csq_00_64 = 1ULL << 5 | 1ULL << 6;
     uint64_t w_csq_000_64 = 1ULL << 2 | 1ULL << 3;

     uint64_t b_csq_00_64 = 1ULL <<1 | 1ULL << 2;
     uint64_t b_csq_000_64 = 1ULL << 4 | 1ULL << 5;

     //we're able to castle, there's nothing there and there's nothing attacking connected castle squares. its then legal
     if(cb.get_whitetoact() )
     {
        if (cb.castling_.getWe_00())
        {
            if (((w_csq_00_64 | 1ULL << 1) & occ) == 0 && ((w_csq_00_64 & attack_mask) == 0)) legal_movelist.add_move(ChessMove(4, 6, 'K', "O-O"));
        }

        if(cb.castling_.getWe_000())
        {
             if((w_csq_000_64 & occ) == 0 && (w_csq_000_64 & attack_mask) == 0) legal_movelist.add_move( ChessMove(4, 2, 'K', "O-O-O"));
        }
     }
     else
    {
        if (cb.castling_.getWe_00())
        {
            if( (b_csq_00_64 & occ) == 0 && (b_csq_00_64 & attack_mask) == 0) legal_movelist.add_move(ChessMove(3, 1, 'K', "O-O"));
        }
        if (cb.castling_.getWe_000())
        {
            if (((b_csq_000_64 | 1ULL << 6) & occ) == 0 && (b_csq_000_64 & attack_mask) == 0) legal_movelist.add_move(ChessMove(3, 5, 'K', "O-O-O"));
        }
     }
    }
 return std::pair<ChessMoveList, int> ( legal_movelist, pp_info.n_attackers );
}

PushPinInfo MoveGenerator::get_push_pin_info(const unsigned long &king, const uint64_t &our_pieces, const uint64_t &enemy_pieces, const uint64_t &enemy_bishops, const uint64_t &enemy_rooks, const uint64_t &enemy_queens)
{
    PushPinInfo pp_info;

    uint64_t push_mask_full {0x0};
    uint64_t push_mask_dir {0x0};
    uint64_t capture_mask {0x0};

    int n_attackers = 0;
    std::vector<std::pair<int, uint64_t>> pins;

    uint64_t r_qs = enemy_queens | enemy_rooks;
    uint64_t b_qs = enemy_queens | enemy_bishops;

    auto intersect_rook_queen = [this, r_qs](const uint64_t & sq_64){return sq_64 & r_qs ;};
    auto intersect_bishop_queen = [this, b_qs](const uint64_t & sq_64){return sq_64 & b_qs ;};

    for(const auto & dir : all_dirs)
    {
        bool is_bishop_queen = dir.first != 0 && dir.second != 0;

        int row = int(king / 8);
        int col = king % 8;

        push_mask_full |= push_mask_dir;
        push_mask_dir = 0x0;

        int pin_at = -1;
        bool found_pin = false;

        while(true)
        {
            row += dir.second;
            col += dir.first;

            if(!on_board(row,col))
            {
                push_mask_dir = 0x0;
                break;
            }

            int idx = row*8 + col;

            uint64_t idx_64 = 1ULL << idx;

            bool found_true = false;

            if(idx_64 & enemy_pieces)
            {
                if (is_bishop_queen && intersect_bishop_queen(idx_64))
                {
                    found_true = true;
                }
                else if(!is_bishop_queen && intersect_rook_queen(idx_64))
                {
                    found_true = true;
                }
                else found_true = false;

                if(!found_true)
                {                    
                    push_mask_dir = 0x0;
                    break;
                }

                if(found_pin)
                {
                    push_mask_dir |= idx_64;
                    pins.emplace_back(std::pair<int, uint64_t>(pin_at, push_mask_dir));
                    push_mask_dir  = 0x0;
                    break;
                }
                else
                {
                    capture_mask |= idx_64;
                    n_attackers += 1;
                    break;
                }
            }
            push_mask_dir |= idx_64;

            if (idx_64 & our_pieces )
            {
                push_mask_dir = 0x0;

                if( found_pin) break;//means we encountered 2 of our own pieces on ray == no pin

                found_pin = true;
                pin_at = idx;
             }
        }
    }
    pp_info.capture_mask = capture_mask;
    pp_info.n_attackers = n_attackers;
    pp_info.pins = pins;
    pp_info.push_mask_full = push_mask_full;

    return pp_info;
}

void MoveGenerator::append_pseudolegal_slidermoves(const ChessBoard &cb,  unsigned long idx, ChessMoveList &movelist, char ptype)
{
    uint64_t atc_64 {0x0};

    if (ptype == 'N')
    {
        atc_64 =get_knight_attacks(idx) & ~cb.get_our_pieces();
    }
    else if (ptype == 'B')
    {
        atc_64 = get_bishop_attacks(cb.get_occ(), idx) & ~cb.get_our_pieces();
    }
    else if (ptype =='R')
    {
        atc_64 = get_rook_attacks(cb.get_occ(), idx) & ~cb.get_our_pieces();
    }
    else if (ptype == 'Q')
    {
        atc_64 =       get_bishop_attacks(cb.get_occ(), idx) & ~cb.get_our_pieces();
        atc_64 |=       get_rook_attacks(cb.get_occ(), idx) & ~cb.get_our_pieces();
    }
    else return ;

    int idx_count = pop_count(atc_64);

    for(int i = 0; i< idx_count; ++i)
    {
        unsigned long lb = least_bit(atc_64);

        movelist.add_move(ChessMove(idx, lb  , ptype));

        atc_64 ^= 1ULL << lb;

        if(atc_64 == 0) break;
    }
}

void MoveGenerator::append_pseudolegal_pawnmoves(const ChessBoard &cb, unsigned long idx, ChessMoveList &movelist)
{
    char ptype = 'P';
    int enp_sq = cb.get_enpassant();

    uint64_t p_atc_64 = get_pawn_attacks(idx) & ~cb.get_enemy_king();

    int idx_count = pop_count(p_atc_64);

    for(int i = 0; i< idx_count; ++i)
    {
        unsigned long lb = least_bit(p_atc_64);

        //do logic with lb right away;
        if (enp_sq == static_cast<int>(lb))
        {
            if (spec_enp_check(cb, lb))movelist.add_move(ChessMove(int(idx), int(lb), ptype, "enp"));
        }
        else if((1ULL << lb) & cb.get_enemy_pieces()) movelist.add_move(ChessMove(idx, lb, ptype));

        p_atc_64 ^= 1ULL << lb;

        if(p_atc_64 == 0) break;
    }

    if(((1ULL << (idx + 8)) & cb.get_occ()) == 0) movelist.add_move(ChessMove(idx, idx + 8, ptype));

    if(idx >= 8 && idx <= 15)
    {
         if(((1ULL << (idx + 8)) & cb.get_occ()) == 0 && ((1ULL << (idx + 16)) & cb.get_occ()) == 0 )
         {
             movelist.add_move(ChessMove(idx , idx  + 16, ptype));
         }
    }
}

bool MoveGenerator::spec_enp_check(const ChessBoard &cb, unsigned long capt_from)
{
    //whatever atm
}

bool MoveGenerator::king_under_attack(const ChessBoard &cb)
{
    uint64_t attacks {0x0};
    uint64_t king_pos{0x0};
    auto occ = cb.get_occ();

    for(int idx = 0 ; idx < 64 ; ++idx)
    {
        if(1ULL << idx & cb.get_king())
        {
            king_pos = 1ULL << idx;
        }
        else if( 1ULL << idx & cb.get_enemy_pawns())
        {
            attacks |= get_pawn_attacks_rev(idx);
        }
        else if( 1ULL << idx & cb.get_enemy_knights())
        {
           attacks |= get_knight_attacks(idx);
        }
        else if( (1ULL << idx) & cb.get_enemy_bishops())
        {
            attacks |= get_bishop_attacks(occ, idx);
        }
        else if( (1ULL << idx) & cb.get_enemy_rooks())
        {
            attacks |= get_rook_attacks(occ, idx);
        }
        else if( (1ULL << idx )& cb.get_enemy_queens())
        {
          attacks |= get_bishop_attacks(occ, idx) | get_rook_attacks(occ, idx);
        }
    }

    return king_pos & attacks;
}

