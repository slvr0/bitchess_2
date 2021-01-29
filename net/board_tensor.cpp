#include "board_tensor.h"

BoardTensor::BoardTensor()
{
    for(int i = 0; i<13;++i) tensor_.emplace_back(0x0);


}

void BoardTensor::reset()
{
    for(int i = 0; i<13;++i)
    {
            unset(i);
    }

}
