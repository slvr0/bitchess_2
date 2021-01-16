#pragma once

#include <iostream>
#include <vector>
#include <x86intrin.h>
#include <cstdint>
#include <stdio.h>
#include <chrono>

constexpr uint64_t row_col_to_idx(const uint64_t& row, const uint64_t& col)
{
    return row*8 + col;
}

template <typename T>
void print(T & v)
{
    std::cout << v << std::endl;
}

template <typename T>
void print(const T & v)
{
    std::cout << v << std::endl;
}

inline std::vector<uint8_t> get_uint64_as_idcs(const uint64_t v)
{
    std::vector<uint8_t> idcs ;
    if (v == 0) return idcs;
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







