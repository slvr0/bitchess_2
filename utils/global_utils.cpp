#include "utils/global_utils.h"

#include <iostream>
#include <vector>


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

template <typename T>
inline bool contains(T v, std::vector<T> indexed_vector)
{
    return std::find(indexed_vector.begin(), indexed_vector.end(), v) != indexed_vector.end();
}




