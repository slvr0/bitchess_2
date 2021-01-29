#include "core/chess_attack_tables.h"

#include "core/chess_board.h"

static uint64_t bishop_attacks_ [5248];
static uint64_t rook_attacks_ [102400];

static bool is_rook_set = false;
static bool is_bishop_set = false;

static uint64_t king_moves_[64];

static uint64_t* rook_lookup_[64];
static uint64_t* bishop_lookup_[64];

uint64_t get_rook_attacks(const uint64_t & occ, const unsigned long idx)
{
    uint64_t occ_c = occ & rook_mask[idx];

    occ_c *= rook_magic_numbers[idx];
    occ_c >>= 64 - rook_shifts[idx];

    return rook_lookup_[idx][occ_c];
}

uint64_t get_bishop_attacks(const uint64_t & occ, const unsigned long idx)
{
    uint64_t occ_c = occ & bishop_mask[idx];

    occ_c *= bishop_magic_numbers[idx];
    occ_c >>= 64 - bishop_shifts[idx];

    return bishop_lookup_[idx][occ_c];
}

uint64_t get_king_moves(unsigned long idx)
{
    return king_moves_[idx];
}
uint64_t get_pawn_attacks_rev( const unsigned long idx)
{
return pawn_attacks_rev[idx];
}
uint64_t get_pawn_attacks(const unsigned long idx)
{
return pawn_attacks[idx];
}
uint64_t get_knight_attacks(const unsigned long idx)
{
return knight_attacks[idx];
}

static void     _init_tables(const std::vector<Direction> &directions, bool rooks)
{
        ChessBoard debug_board;

        int nr_filled = 0;
        int offset  = 0;

        auto onboard = [](int row, int col) { return row >= 0 && row < 8 && col >= 0 && col <8;};

        for(int idx = 0 ; idx < 64; idx++)
        {
            std::vector<unsigned long> BB_indices;

            if(rooks)BB_indices = bit_iter(rook_mask[idx]);
            else BB_indices =  bit_iter(bishop_mask[idx]);

            for(int i = 0 ; i < 1ULL << BB_indices.size(); ++i)
            {
                uint64_t occupation_mask{0x0};

                for(int k = 0; k < BB_indices.size() ; ++k)
                {
                    if((1 << k) & i)
                    {
                        occupation_mask |= 1ULL << BB_indices.at(k);
                    }
                }

                //set lookup table to the correct memory adress in attack tables
                if(rooks) rook_lookup_[idx] = &rook_attacks_[offset];
                else bishop_lookup_[idx] = &bishop_attacks_[offset];

                uint64_t sq_to_bb{0x0};

                for(const auto & dir : directions)
                {
                    int row = int(idx/8);
                    int col = int(idx % 8);

                    while (true)
                    {
                        row += dir.second;
                        col += dir.first;

                        if (!onboard(row, col)) break;

                        uint64_t ns_64 {1ULL <<  (row*8 + col)};

                        sq_to_bb |= ns_64;

                        if ((ns_64 & occupation_mask) != 0)  break;
                    }
                }

                if(rooks)
                {
                    occupation_mask *= rook_magic_numbers[idx];
                    occupation_mask >>= 64 - rook_shifts[idx];
                }
                else
                {
                    occupation_mask *= bishop_magic_numbers[idx];
                    occupation_mask >>= 64 - bishop_shifts[idx];
                }

                int n_idx = int(offset + occupation_mask);

                if(rooks && occupation_mask >1ULL <<  rook_shifts[idx]) print("sanity check wrong on rook indexing \n");
                else if(!rooks && occupation_mask > 1ULL <<  bishop_shifts[idx]) print("sanity check wrong on bishop indexing \n");

                if(rooks) rook_attacks_[n_idx] = sq_to_bb;
                else bishop_attacks_[n_idx] = sq_to_bb;

                nr_filled++;
            }
        }

        std::string type = rooks ? "Rooks" : "Bishops";
        std::cout << type << " attacks init. " << nr_filled << " sequences inserted \n";

        if(rooks) is_rook_set = true;
        else is_bishop_set = true;
}

static void  _init_king_tables()
{
        std::vector<Direction> kd {Direction(1, 1), Direction(1, -1), Direction(-1, 1), Direction(-1, -1), Direction(1, 0), Direction(-1,0), Direction(0,1), Direction(0, -1)};

       for(int i = 0; i < 64 ; ++i)
       {
           uint64_t kmoves {0x0};

           for(const auto & dir : kd)
           {
               int row =int(i / 8) ;
               int col = (i % 8) ;

               row += dir.second;
               col += dir.first;

               if(on_board(row,col)) kmoves |= 1ULL << (row*8 +col);
           }

           king_moves_[i] = kmoves;
        }
}

void init_tables()
{
        std::vector<Direction> bd {Direction(1, 1), Direction(1, -1), Direction(-1, 1), Direction(-1, -1)};
        std::vector<Direction> rd {Direction(1, 0), Direction(-1,0), Direction(0,1), Direction(0, -1)};

        _init_tables(bd, false);
        _init_tables(rd, true);

        _init_king_tables();

}
