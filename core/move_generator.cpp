#include "move_generator.h"

#include "utils/global_utils.cpp"
#include "core/chess_attack_tables.h"
#include "core/chess_move.h"

MoveGenerator::MoveGenerator()
{
    
}

ChessMoveList MoveGenerator::get_legal_moves(const ChessBoard &cb)
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

    uint64_t king_moves{0x0}; //this can also just be indexed, havent found a table for this yet

     //could do bit iter over occ here instead. try it later

    for(int idx = 0 ; idx < 64 ; ++idx)
    {
        if (1ULL << idx & cb.get_king())
        {
            king_moves = get_king_moves(idx);
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
            uint64_t attacks = pawn_attacks_rev[idx];
            if (attacks & cb.get_king())
            {
                non_slider_attackers++;
            }
            attack_mask |= attacks;
            attack_mask_noking |= attacks;
        }
        else if( 1ULL << idx & cb.get_enemy_knights())
        {
          uint64_t attacks = knight_attacks[idx];
          attack_mask |= attacks;
          attack_mask_noking |= attacks;

          if (attacks & cb.get_king())
          {
              knight_capture_mask |= 1ULL << idx;
              non_slider_attackers++;
          }
        }
        else if( 1ULL << idx & cb.get_enemy_bishops())
        {
            uint64_t attacks = get_bishop_attacks(occ, idx);
            uint64_t attacks_noking = get_bishop_attacks(occ - cb.get_king(), idx);
            attack_mask |= attacks;
            attack_mask_noking |= attacks_noking;
        }
        else if( 1ULL << idx & cb.get_enemy_rooks())
        {
            uint64_t attacks = get_rook_attacks(occ, idx);
            uint64_t attacks_noking = get_rook_attacks(occ - cb.get_king(), idx);
            attack_mask |= attacks;
            attack_mask_noking |= attacks_noking;
        }
        else if( 1ULL << idx & cb.get_enemy_queens())
        {
          uint64_t attacks = get_bishop_attacks(occ, idx) | get_rook_attacks(occ, idx);
          uint64_t attacks_noking = get_bishop_attacks(occ - cb.get_king(), idx) | get_rook_attacks(occ - cb.get_king(), idx);
          attack_mask |= attacks;
          attack_mask_noking |= attacks_noking;
        }

        else if( 1ULL << idx & cb.get_enemy_king())
        {
            uint64_t  attacks = get_king_moves(idx);
            attack_mask |= attacks;
            attack_mask_noking |= attacks;
        }
    }

    pp_info.n_attackers += non_slider_attackers;
    pp_info.capture_mask |= knight_capture_mask;

    king_moves &= ~our_pieces;
    king_moves &= ~attack_mask_noking;
    king_moves &= ~cb.get_enemy_king();

    //finally remove movement to protected enemy pieces
    king_moves &= ~(attack_mask_noking & enemy_pieces);

    auto k_move_idcs = bit_iter(king_moves);

   ChessMoveList legal_movelist;

   for(const auto & move : k_move_idcs)  legal_movelist.add_move(ChessMove(king_idx, move, 'K'));

   if(pp_info.n_attackers > 1) return legal_movelist;

   //now check which moves are legal or not based on the Pin Push Capture information collected

   int m_idx = 0;
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
 return legal_movelist;
}

uint64_t MoveGenerator::get_king_moves(unsigned long idx)
{

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
                if (is_bishop_queen && intersect_bishop_queen(idx_64))  found_true = true;
                else if(!is_bishop_queen && intersect_rook_queen(idx_64)) found_true = true;
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
     return pp_info;
}

void MoveGenerator::append_pseudolegal_slidermoves(const ChessBoard &cb,  unsigned long idx, ChessMoveList &movelist, char ptype)
{
    uint64_t atc_64 {0x0};

    if (ptype == 'N')
    {
        atc_64 = knight_attacks[idx] & ~(cb.get_our_pieces() | cb.get_enemy_king());
    }
    else if (ptype == 'B')
    {
        atc_64 =       get_bishop_attacks(cb.get_occ(), idx) & ~(cb.get_our_pieces() | cb.get_enemy_king());
    }
    else if (ptype =='R')
    {
        atc_64 =       get_rook_attacks(cb.get_occ(), idx) & ~(cb.get_our_pieces() | cb.get_enemy_king());
    }
    else if (ptype == 'Q')
    {
        atc_64 =       get_bishop_attacks(cb.get_occ(), idx) & ~(cb.get_our_pieces() | cb.get_enemy_king());
        atc_64 |=       get_rook_attacks(cb.get_occ(), idx) & ~(cb.get_our_pieces() | cb.get_enemy_king());
    }

    else return ;

    for(const auto & square : bit_iter(atc_64))
    {
        movelist.add_move(ChessMove(idx, square, ptype));
    }
}

void MoveGenerator::append_pseudolegal_pawnmoves(const ChessBoard &cb, unsigned long idx, ChessMoveList &movelist)
{
    char ptype {'P'};
    int enp_sq = cb.get_enpassant();
    uint64_t p_atc_64 = pawn_attacks[idx] & ~cb.get_enemy_king();
    auto p_sq_idcs = bit_iter(p_atc_64);

    for (const auto & square : p_sq_idcs)
    {
        if (enp_sq == square)
        {
            if (spec_enp_check(cb, square))movelist.add_move(ChessMove(int(idx), int(square), ptype, "enp"));
        }
        else if(1ULL << square & cb.get_enemy_pieces()) movelist.add_move(ChessMove(square, enp_sq, ptype));
    }

    if((1ULL << (idx + 8) & cb.get_occ()) == 0) movelist.add_move(ChessMove(idx, idx + 8, ptype));

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

}
