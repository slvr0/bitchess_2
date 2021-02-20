#include "chess_castle.h"

Castling::Castling() :
    we_00(true),
    we_000(true),
    enemy_00(true),
    enemy_000(true)
{

}

void Castling::mirror()
{
    auto tmp_00 = getWe_00();
    auto tmp_000 = getWe_000();

    setWe_00(getEnemy_00());
    setWe_000(getEnemy_000());

    setEnemy_00(tmp_00);
    setEnemy_000(tmp_000);
}

bool Castling::getWe_00() const
{
    return we_00;
}

void Castling::setWe_00(bool value)
{
    we_00 = value;
}

bool Castling::getWe_000() const
{
    return we_000;
}

void Castling::setWe_000(bool value)
{
    we_000 = value;
}

bool Castling::getEnemy_00() const
{
    return enemy_00;
}

void Castling::setEnemy_00(bool value)
{
    enemy_00 = value;
}

bool Castling::getEnemy_000() const
{
    return enemy_000;
}

void Castling::setEnemy_000(bool value)
{
    enemy_000 = value;
}

void Castling::update_castlestatus(const ChessMove & move, const int & white_toact)
{
    char pt = move.ptype();

    if (pt == 'K' )
    {
      setWe_00(false);
      setWe_000(false);
      return;
    }
    if (pt == 'R' )
    {
      if (move.from() == 0)
      {
        if (white_toact == 1) setWe_000(false);
        else setWe_00(false);
      }
      else if(move.from() == 7)
      {
        if (white_toact == 1) setWe_00(false);
        else setWe_000(false);
      }
    }

    if  (move.to() == 63)  setEnemy_00(false);
    if  (move.to() == 56)  setEnemy_000(false);
}
