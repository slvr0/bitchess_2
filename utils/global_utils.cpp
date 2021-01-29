#pragma once

#include <iostream>
#include <vector>
#include <x86intrin.h>
#include <cstdint>
#include <stdio.h>
#include <chrono>

#include <stdlib.h>
#include <time.h>
#include <map>

static std::map<char,int> ptypelist = { {'P',0},   {'N',1},  {'B',2}, {'R',3}, {'Q',4}, {'K',5}, {'p',6}, {'n',7}, {'b',8}, {'r',9}, {'q',10}, {'k',11}};

constexpr uint64_t row_col_to_idx(const uint64_t& row, const uint64_t& col)
{
    return row*8 + col;
}

inline bool on_board(const int & row, const int & col){return   row >= 0 && row < 8 && col >= 0 && col < 8;}



template <typename T>
void print(T & v)
{
    std::cout << v << std::endl;
}

inline uint64_t reverse_bytes_in_byte(const uint64_t&  x)
{
      uint64_t v = x;
      v = (v & 0x00000000FFFFFFFF) << 32 | (v & 0xFFFFFFFF00000000) >> 32;
      v = (v & 0x0000FFFF0000FFFF) << 16 | (v & 0xFFFF0000FFFF0000) >> 16;
      v = (v & 0x00FF00FF00FF00FF) << 8 | (v & 0xFF00FF00FF00FF00) >> 8;
      return v;
}

inline uint64_t reverse_bits_in_byte(uint64_t x) {
uint64_t v = x;
v = ((v >> 1) & 0x5555555555555555ull) | ((v & 0x5555555555555555ull) << 1);
v = ((v >> 2) & 0x3333333333333333ull) | ((v & 0x3333333333333333ull) << 2);
v = ((v >> 4) & 0x0F0F0F0F0F0F0F0Full) | ((v & 0x0F0F0F0F0F0F0F0Full) << 4);
return v;
}


inline uint64_t transpose_bits_in_byes(const uint64_t&  x)
{
     uint64_t v = x;
  v = (v & 0xAA00AA00AA00AA00ULL) >> 9 | (v & 0x0055005500550055ULL) << 9 |
      (v & 0x55AA55AA55AA55AAULL);
  v = (v & 0xCCCC0000CCCC0000ULL) >> 18 | (v & 0x0000333300003333ULL) << 18 |
      (v & 0x3333CCCC3333CCCCULL);
  v = (v & 0xF0F0F0F000000000ULL) >> 36 | (v & 0x000000000F0F0F0FULL) << 36 |
      (v & 0x0F0F0F0FF0F0F0F0ULL);
  return v;
}

template <typename T>
void print(const T & v)
{
    std::cout << v << std::endl;
}

inline bool contains(const char& v, const std::string & indexed_string)
{
    return indexed_string.find(v) != std::string::npos;
}

inline unsigned long pop_count(const uint64_t& v )
{
    return __builtin_popcountll(v);
}

inline unsigned long least_bit(const uint64_t &v)
{
    return __builtin_ctzll(v);
}

template <typename T>
inline bool contains(T v, std::vector<T> indexed_vector)
{
    return std::find(indexed_vector.begin(), indexed_vector.end(), v) != indexed_vector.end();
}

inline void bit_iter_fast(const uint64_t &v )
{
     uint64_t it_t = v;
     int idx_count = pop_count(it_t);

     int shifts = 0;
     for(int i = 0; i< idx_count; ++i)
     {
         unsigned long lb = least_bit(it_t);
         it_t >>= lb + 1;
         shifts += lb + 1;

         //do logic with lb right away;

         if(it_t == 0) break;
     }
}

//slow! dont use in move gen
inline std::vector<unsigned long> bit_iter(const uint64_t & v)
{
    uint64_t it_t = v;
    std::vector<unsigned long> res;

    int idx_count = pop_count(it_t);

    int shifts = 0;

    for(int i = 0 ; i<idx_count;++i)
    {
        unsigned long lb = least_bit(it_t);

        res.emplace_back(lb + shifts);
        it_t >>= lb + 1;
        shifts += lb + 1;

        if((it_t) == 0)
            return res;
    }

    return res;
}

class Timer
{
public :
    using clock_t = std::chrono::high_resolution_clock;
    using second_t = std::chrono::duration<double, std::ratio<1> >;

    Timer() : time_(clock_t::now())
    {

    }

    double elapsed()
    {
        return std::chrono::duration_cast<second_t>(clock_t::now() - time_).count();
    }

    void reset()
    {
        time_ = clock_t::now();
    }
private:
    std::chrono::time_point<clock_t> time_;

};







