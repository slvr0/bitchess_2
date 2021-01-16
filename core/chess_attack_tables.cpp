#include "chess_attack_tables.h"
#include <cmath>

#include <memory>

#include "utils/global_utils.cpp"

ChessAttackTables::ChessAttackTables()
{

}

void ChessAttackTables:: init_tables()
{
    std::vector<Direction> bd {Direction(1, 1), Direction(1, -1), Direction(-1, 1), Direction(-1, -1)};
    std::vector<Direction> rd {Direction(1, 0), Direction(-1,0), Direction(0,1), Direction(0, -1)};

    init_attack_tables(bd, false);
    init_attack_tables(rd, true);
}

void ChessAttackTables::init_attack_tables(const std::vector<Direction> & directions, bool rooks)
{
    int nr_filled = 0;
    int offset  = 0;

    auto onboard = [](int row, int col) { return row >= 0 && row < 8 && col >= 0 && col <8;};

    for(int idx = 0 ; idx < 64; idx++)
    {
         if(rooks) rook_offsets_[idx] = offset;
         else bishop_offsets_[idx] = offset;



        std::vector<unsigned long> BB_indices;

        if(rooks) BB_indices = bit_iter(rook_mask[idx]);
        else BB_indices =  bit_iter(bishop_mask[idx]);

        for(uint64_t i = 0 ; i < 1ULL << BB_indices.size(); ++i)
        {
            uint64_t occupation_mask{0x0};

            for(int k = 0; k < BB_indices.size() ; ++k)
            {
                if (1ULL << k & i ) occupation_mask |= 1ULL << BB_indices.at(k);
            }

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

                    int ns_64 = 1ULL <<  (row*8 + col);

                    sq_to_bb |= ns_64;

                    if (ns_64 & occupation_mask)  break;
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

        if(rooks) offset += 1ULL <<  rook_shifts[idx];
        else  offset +=  1ULL <<  bishop_shifts[idx];
    }

    std::string type = rooks ? "Rooks" : "Bishops";
    std::cout << type << " attacks init. " << nr_filled << " sequences inserted \n";
}

uint64_t ChessAttackTables::get_rook_attacks(const uint64_t &occ, uint8_t idx)
{

}

uint64_t ChessAttackTables::get_bishop_attacks(const uint64_t &occ, uint8_t idx)
{

}
