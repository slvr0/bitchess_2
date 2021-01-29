#include "core/zobr_hash.h"

#include <stdlib.h>
#include <time.h>

static uint64_t hash_[64][12];

void  init_prehash()
{
    srand(time(NULL));

    for(int i = 0 ; i < 64 ; ++i)
    {
        for(int k = 0; k < 12; ++k)
        {
            hash_[i][k] = rand() % (1ULL << 63);
        }
    }
    std::cout<< "zobrist prehash values initiated \n";
}

uint64_t get_hash(int row, int col)
{
    return hash_[row][col];
}
