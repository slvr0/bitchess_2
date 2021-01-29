#pragma once

#include <iostream>
#include <vector>

class BoardTensor
{
public:
    BoardTensor();

    inline void set(int type, uint64_t v)
    {
        tensor_.at(type) = v;
    }

    inline void unset(int type)
    {
         tensor_.at(type) = 0x0;
    }

    void reset();

    inline uint64_t get(int type)
    {
        return tensor_.at(type);
    }

private:
    std::vector<uint64_t> tensor_;
};

