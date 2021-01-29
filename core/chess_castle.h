#pragma once

#include "core/chess_move.h"

class Castling
{
public:
    Castling();

    void mirror();

    bool getWe_00() const;
    void setWe_00(bool value);

    bool getWe_000() const;
    void setWe_000(bool value);

    bool getEnemy_00() const;
    void setEnemy_00(bool value);

    bool getEnemy_000() const;
    void setEnemy_000(bool value);

    void update_castlestatus(const ChessMove & move, const int &white_toact);


private :
    bool we_00;
    bool we_000;
    bool enemy_00;
    bool enemy_000;



};

