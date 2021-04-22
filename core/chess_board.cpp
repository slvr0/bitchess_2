#include "chess_board.h"

#include <vector>
#include <algorithm>
#include <sstream>
#include <string>

#include "utils/global_utils.cpp"
#include "core/zobr_hash.h"

#include <exception>

ChessBoard::ChessBoard() :
    rule50_(0),
    total_moves_(0),
    three_repeat_(0),
    white_toact_(1),
    enpassant_(-1)
{

}

ChessBoard::ChessBoard(std::string fen_string) :
    rule50_(0),
    total_moves_(0),
    three_repeat_(0),
    white_toact_(1),
    enpassant_(-1)
{    
    set_from_fen(fen_string);
}

void ChessBoard::reset()
{
    pawns_ = 0x0;
    bishops_ = 0x0;
    knights_ = 0x0;
    rooks_ = 0x0;
    queens_ = 0x0;
    king_ = 0x0;

    enemy_pawns_ = 0x0;
    enemy_bishops_ = 0x0;
    enemy_knights_ = 0x0;
    enemy_rooks_ = 0x0;
    enemy_queens_ = 0x0;
    enemy_king_ = 0x0;

    our_pieces_  = 0x0;
    enemy_pieces_  = 0x0;
}

//swap sides // flip vertical and horizontal
void ChessBoard::mirror()
{
    pawns_ = reverse_bytes_in_byte(reverse_bits_in_byte(pawns_));
    bishops_ =  reverse_bytes_in_byte(reverse_bits_in_byte(bishops_));
    knights_ = reverse_bytes_in_byte( reverse_bits_in_byte(knights_));
    rooks_ =  reverse_bytes_in_byte(reverse_bits_in_byte(rooks_));
    queens_ =  reverse_bytes_in_byte(reverse_bits_in_byte(queens_));
    king_ =  reverse_bytes_in_byte(reverse_bits_in_byte(king_));

    enemy_pawns_ =  reverse_bytes_in_byte(reverse_bits_in_byte(enemy_pawns_));
    enemy_bishops_ =  reverse_bytes_in_byte(reverse_bits_in_byte(enemy_bishops_));
    enemy_knights_ =  reverse_bytes_in_byte(reverse_bits_in_byte(enemy_knights_));
    enemy_rooks_ =  reverse_bytes_in_byte(reverse_bits_in_byte(enemy_rooks_));
    enemy_queens_ = reverse_bytes_in_byte(reverse_bits_in_byte(enemy_queens_));
    enemy_king_ =  reverse_bytes_in_byte(reverse_bits_in_byte(enemy_king_));

    enemy_pieces_  = reverse_bytes_in_byte(reverse_bits_in_byte(enemy_pieces_));
    our_pieces_ = reverse_bytes_in_byte( reverse_bits_in_byte(our_pieces_));

    std::swap(pawns_, enemy_pawns_);
    std::swap(knights_, enemy_knights_);
    std::swap(bishops_, enemy_bishops_);
    std::swap(rooks_, enemy_rooks_);
    std::swap(queens_, enemy_queens_);

    std::swap(king_, enemy_king_);
    std::swap(our_pieces_, enemy_pieces_);

    white_toact_ = white_toact_ == 1 ? 0 : 1;

    if(enpassant_ != -1)
    {
        enpassant_ = 63 - enpassant_;
    }

    castling_.mirror();

    set_zobrist();
}

void ChessBoard::add_piece(const uint8_t &square, char ptype)
{
    if (ptype == 'P')pawns_ |= 1ULL << square;
    else if( ptype == 'N')knights_ |= 1ULL << square;
    else if( ptype == 'B') bishops_ |= 1ULL << square;
    else if( ptype == 'R') rooks_ |= 1ULL << square;
    else if( ptype == 'Q') queens_ |= 1ULL << square;
    else if( ptype == 'K') king_ |= 1ULL << square;
    else if( ptype == 'p') enemy_pawns_ |= 1ULL << square;
    else if( ptype == 'n') enemy_knights_ |= 1ULL << square;
    else if( ptype == 'b') enemy_bishops_ |= 1ULL << square;
    else if( ptype == 'r')enemy_rooks_ |= 1ULL << square;
    else if( ptype == 'q') enemy_queens_ |= 1ULL << square;
    else if( ptype == 'k') enemy_king_ |= 1ULL << square;

    if(isupper(ptype)) our_pieces_ |= 1ULL << square;
    else enemy_pieces_ |= 1ULL << square;
}

void ChessBoard::remove_piece(const uint8_t &square)
{
    uint64_t sq_64 = 1ULL << square;

    pawns_ &= ~sq_64;
    bishops_ &= ~sq_64;
    knights_ &=~ sq_64;
    rooks_ &= ~sq_64;
    queens_ &= ~sq_64;
    king_ &= ~sq_64;
    enemy_pawns_ &= ~sq_64;
    enemy_bishops_ &=~sq_64;
    enemy_knights_ &= ~sq_64;
    enemy_rooks_ &= ~sq_64;
    enemy_queens_ &= ~sq_64;
    enemy_king_ &= ~sq_64;

    our_pieces_ &= ~sq_64;
    enemy_pieces_ &= ~sq_64;
}

std::pair<char, uint64_t> ChessBoard::occupied_by(const short &s_idx) const
{
    const uint64_t s_idx_64 = 1ULL << s_idx;

    if (pawns_ & s_idx_64) return std::pair<char, uint64_t> ('P', pawns_);
    else if (enemy_pawns_ & s_idx_64) return std::pair<char, uint64_t> ('p', enemy_pawns_);

    else if (knights_ & s_idx_64) return std::pair<char, uint64_t> ('N', knights_);
    else if (bishops_ & s_idx_64) return std::pair<char, uint64_t> ('B', bishops_);
    else if (rooks_ & s_idx_64) return std::pair<char, uint64_t> ('R', rooks_);
    else if (queens_ & s_idx_64) return std::pair<char, uint64_t> ('Q', queens_);
    else if (king_ & s_idx_64 ) return std::pair<char, uint64_t> ('K', king_);

    else if (enemy_knights_ & s_idx_64) return std::pair<char, uint64_t> ('n', enemy_knights_);
    else if (enemy_bishops_ & s_idx_64) return std::pair<char, uint64_t> ('b', enemy_bishops_);
    else if (enemy_rooks_ & s_idx_64) return std::pair<char, uint64_t> ('r', enemy_rooks_);
    else if (enemy_queens_ & s_idx_64) return std::pair<char, uint64_t> ('q', enemy_queens_);
    else if (enemy_king_ & s_idx_64) return std::pair<char, uint64_t> ('k', enemy_king_);

    else return std::pair<char, uint64_t> (',', 0);
}

std::pair<int, uint64_t> ChessBoard::occupied_by_res_int(const short &s_idx) const
{
    const uint64_t s_idx_64 = 1ULL << s_idx;

    if (pawns_ & s_idx_64) return std::pair<int, uint64_t> (0, pawns_);
    else if (enemy_pawns_ & s_idx_64) return std::pair<int, uint64_t> (6, enemy_pawns_);

    else if (knights_ & s_idx_64) return std::pair<int, uint64_t> (1, knights_);
    else if (bishops_ & s_idx_64) return std::pair<int, uint64_t> (2, bishops_);
    else if (rooks_ & s_idx_64) return std::pair<int, uint64_t> (3, rooks_);
    else if (queens_ & s_idx_64) return std::pair<int, uint64_t> (4, queens_);
    else if (king_ & s_idx_64 ) return std::pair<int, uint64_t> (5, king_);

    else if (enemy_knights_ & s_idx_64) return std::pair<int, uint64_t> (7, enemy_knights_);
    else if (enemy_bishops_ & s_idx_64) return std::pair<int, uint64_t> (8, enemy_bishops_);
    else if (enemy_rooks_ & s_idx_64) return std::pair<int, uint64_t> (9, enemy_rooks_);
    else if (enemy_queens_ & s_idx_64) return std::pair<int, uint64_t> (10, enemy_queens_);
    else if (enemy_king_ & s_idx_64) return std::pair<int, uint64_t> (11, enemy_king_);

    else return std::pair<int, uint64_t> (-1, 0x0);
}

bool ChessBoard::has_mating_chance() const
{
    if((rooks_|enemy_rooks_) || (pawns_|enemy_pawns_) || (queens_|enemy_queens_)) return true;

    if(pop_count(our_pieces_ | enemy_pieces_) < 4 ) return false;

    if(knights_ | enemy_knights_) return true;

    uint64_t lq_bishop =  0x55AA55AA55AA55AAULL;
    uint64_t dq_bishop = 0xAA55AA55AA55AA55ULL;

    uint64_t lq_at = (bishops_ | enemy_bishops_) & lq_bishop;
    uint64_t dq_at = (bishops_ | enemy_bishops_) & dq_bishop;

    return lq_at || dq_at;
}

void ChessBoard::update_from_move(const ChessMove &move)
{
    int from = move.from();
    int to = move.to();
    char ptype = move.ptype();

    std::string spec_action = move.spec_action();
    char promotion = move.promotion();

    if (spec_action == "enp")
    {
        add_piece(to, ptype);
        remove_piece(from);
        remove_piece(to - 8);
        rule50_ = 0;
    }
    else if( spec_action == "O-O")
    {
        if (white_toact_)
        {
            int king_on = 4;
            int rook_on = 7;
            remove_piece(king_on);
            remove_piece(rook_on);
            add_piece(6, 'K');
            add_piece(5, 'R');
        }
        else
        {
            int king_on = 3;
            int rook_on = 0;
            remove_piece(king_on);
            remove_piece(rook_on);
            add_piece(1, 'K');
            add_piece(2, 'R');
        }
    }
    else if (spec_action == "O-O-O")
    {
        if(white_toact_)
        {
            int king_on = 4;
            int rook_on = 0;
            remove_piece(king_on);
            remove_piece(rook_on);
            add_piece(2, 'K');
            add_piece(3, 'R');
        }
        else
        {
            int king_on = 3;
            int rook_on = 7;
            remove_piece(king_on);
            remove_piece(rook_on);
            add_piece(5, 'K');
            add_piece(4, 'R');
        }
    }
    else if (promotion != ' ')
    {
        remove_piece(to);
        remove_piece(from);
        add_piece(to, promotion);
    }
    else
    {
        remove_piece(to);
        remove_piece(from);
        add_piece(to, ptype);
    }

   total_moves_ += 1;
   rule50_ = ptype != 'P'  ? rule50_ + 1 : 0;
   enpassant_ = -1;

    if (ptype == 'P' && from >= 8 && from < 16)
    {
      int dm = to - from;
      if( dm == 16) enpassant_ = to - 8;
    }

    castling_.update_castlestatus(move, white_toact_);

    set_zobrist();
}

void ChessBoard::set_zobrist()
{
    uint64_t zv {0x0};

    for(int i = 0; i < 64 ; ++i)
    {
       std::pair<char, uint64_t> occ_pair =  occupied_by(i);

       if(occ_pair.first == -1) continue;

       zv ^= get_hash(i, occ_pair.first);

//       if(white_toact_)
//       {
//           zv ^= get_hash(i, occ_pair.first);
//       }
//       else
//       {
//           zv ^= get_hash(63-i,occ_pair.first);
//       }
    }

     z_hash_ = zv;
}

void ChessBoard::update_zobrist(const ChessMove &move)
{
    int from = move.from();
    int to = move.to();
    char ptype = move.ptype();
    std::string spec_action = move.spec_action();
    char promo = move.promotion();

    if (promo == ' ' and spec_action == "")
    {
        z_hash_ ^= get_hash(from,ptypelist.at(ptype));
        z_hash_ ^= get_hash(to,ptypelist.at(ptype));
        return ;
    }

    else if( promo != ' ')
    {
        z_hash_ ^= get_hash(from,ptypelist.at(ptype));
        z_hash_ ^= get_hash(to,ptypelist.at(promo));
        return ;
    }

    else if (spec_action == "O-O")
    {
        z_hash_ ^= get_hash(7,ptypelist['R']);
        z_hash_ ^= get_hash(4,ptypelist['K']);

        z_hash_ ^= get_hash(5,ptypelist['R']);
        z_hash_ ^= get_hash(6,ptypelist['K']);
        return;
    }
    else if(spec_action == "O-O-O")
    {
        z_hash_ ^= get_hash(0,ptypelist['R']);
        z_hash_ ^= get_hash(4,ptypelist['K']);

        z_hash_ ^= get_hash(3,ptypelist['R']);
        z_hash_ ^= get_hash(2,ptypelist['K']);
        return;
    }
    else if(spec_action == "enp")
    {
        z_hash_ ^= get_hash(from,ptypelist.at(ptype));
        z_hash_ ^= get_hash(to,ptypelist.at(ptype));
        z_hash_ ^= get_hash(to-8,ptypelist.at(ptype));
        return;
    }
}

void ChessBoard::check_castle_legality()
{
    if(white_toact_)
    {
        if((king_ & 1ULL << 4) == 0)
        {
            //disallow castling completely
            castling_.setWe_00(false);
            castling_.setWe_000(false);
        }
        if((enemy_king_ & 1ULL << 60) == 0)
        {
            //disallow castling completely
            castling_.setEnemy_00(false);
            castling_.setEnemy_000(false);
        }
        if((rooks_ & 1ULL << 7) == 0)
        {
            castling_.setWe_00(false);
        }
        if((rooks_ & 1ULL << 0) == 0)
        {
            castling_.setWe_000(false);
        }
        if((enemy_rooks_ & 1ULL <<63) == 0)
        {
            castling_.setEnemy_00(false);
        }
        if((enemy_rooks_ & 1ULL << 56) == 0)
        {
            castling_.setEnemy_000(false);
        }
    }
    else
    {
        if((king_ & 1ULL << 3) == 0)
        {
            //disallow castling completely
            castling_.setWe_00(false);
            castling_.setWe_000(false);
        }
        if((enemy_king_ & 1ULL << 59) == 0)
        {
            //disallow castling completely
            castling_.setEnemy_00(false);
            castling_.setEnemy_000(false);
        }

        if((rooks_ & 1ULL << 7) == 0)
        {
            castling_.setWe_000(false);
        }
        if((rooks_ & 1ULL << 0) == 0)
        {
            castling_.setWe_00(false);
        }
        if((enemy_rooks_ & 1ULL <<63) == 0)
        {
            castling_.setEnemy_000(false);
        }
        if((enemy_rooks_ & 1ULL << 56) == 0)
        {
            castling_.setEnemy_00(false);
        }
    }
}

std::string ChessBoard::fen() const
{

    std::string fen;

    int voids_count = 0;

    for(int r =7; r >= 0 ; r--)
    {
        for(int c = 0 ; c < 8; ++c)
        {
            int idx = r*8+c;
            idx = white_toact_ ? idx : 63 - idx;
            auto space_occ = this->occupied_by(idx).first;

            if(space_occ == ',') voids_count++;
            else
            {
                if(voids_count != 0) fen.append(std::to_string(voids_count));

                //remember , colors flipped for black, switch upper/lower case
                if(white_toact_ == 0 )
                {
                    if(isupper(space_occ)) space_occ = tolower(space_occ);
                    else space_occ = toupper(space_occ);
                }

                fen.push_back(space_occ);
                voids_count = 0;
            }
        }

    if(voids_count != 0) fen.append(std::to_string(voids_count));
    voids_count = 0;
    if(r>=1) fen.push_back('/');
    }

    //to act
    fen.append(white_toact_ ? " w" : " b");

    //castling state (remember, this is mirrored so KQ if its black turn should be noted as kq.
    bool we_00 = castling_.getWe_00();
    bool we_000 = castling_.getWe_000();
    bool e_00 = castling_.getEnemy_00();
    bool e_000 = castling_.getEnemy_000();

    std::string castle_string = " ";

    if (white_toact_)
    {
        if(we_00) castle_string.push_back('Q');
        if(we_000) castle_string.push_back('K');
        if(e_00) castle_string.push_back('q');
        if(e_000) castle_string.push_back('k');
    }
    else
    {
        if(e_00) castle_string.push_back('Q');
        if(e_000) castle_string.push_back('K');
        if(we_00) castle_string.push_back('q');
        if(we_000) castle_string.push_back('k');
    }
    if(!we_00 && !we_000 && !e_00 && !e_000)fen.append(" -");
    else fen.append(castle_string);

    //enpassant, simple dash if no square (-)

    std::string enp_not;
    if(enpassant_ == -1) enp_not = " -";
    else
    {
        if(white_toact_ ) enp_not = " " +board_notations[enpassant_];
        else enp_not = " " +board_notations[63 - enpassant_];

        std::transform(enp_not.begin(), enp_not.end(),enp_not.begin(), tolower);
    }

    fen.append(enp_not);

    //50 rule count, for no pawn movements
    fen.append(" " + std::to_string(this->rule50_));

    //move count total
     fen.append(" " + std::to_string(this->total_moves_));

    return fen;
}

void ChessBoard::set_from_fen(std::string fen_string)
{

        int row = 7;
        int col = 0;

        // Remove any trailing whitespaces to detect eof after the last field.
        fen_string.erase(std::find_if( fen_string.rbegin(), fen_string.rend(), [](char c) { return !std::isspace(c); } ).base(),
                         fen_string.end());

        std::istringstream fen_str(fen_string);
        std::string board;

        fen_str >> board;
        std::string who_to_move = "w";
        if (!fen_str.eof()) fen_str >> who_to_move;
        std::string castlings = "-";
        if (!fen_str.eof()) fen_str >> castlings;
        std::string en_passant = "-";
        if (!fen_str.eof()) fen_str >> en_passant;
        int rule50 = 0;
        if (!fen_str.eof()) fen_str >> rule50;
        int total_moves = 1;
        if (!fen_str.eof()) fen_str >> total_moves;

        for (char c : board) {

          if (c == '/') {
            --row;
            col = 0;
            continue;
          }
          if (std::isdigit(c)) {
            col += c - '0';
            continue;
          }

          uint64_t idx =  row_col_to_idx(row,col);


          if (std::isupper(c)) {
            // White piece.
            our_pieces_ |= 1ULL << idx;
          } else {
            // Black piece.
            enemy_pieces_|= 1ULL << idx;
          }

        if (c == 'P') pawns_ |=  1ULL << idx;
        else if (c == 'N') knights_ |= 1ULL << idx;
        else if (c == 'B') bishops_ |=1ULL << idx;
        else if (c == 'R') rooks_ |= 1ULL << idx;
        else if (c == 'Q') queens_ |= 1ULL << idx;
        else if (c == 'K')  king_|= 1ULL << idx;
        else if (c == 'p') enemy_pawns_ |=1ULL << idx;
        else if (c == 'n') enemy_knights_ |=1ULL << idx;
        else if (c == 'b') enemy_bishops_ |=1ULL << idx;
        else if (c == 'r') enemy_rooks_ |= 1ULL << idx;
        else if (c == 'q') enemy_queens_ |=1ULL << idx;
        else if (c == 'k') enemy_king_ |= 1ULL << idx;

        ++col;
        }        

        rule50_ = rule50;
        total_moves_ = total_moves;

        if(en_passant == "-") enpassant_ = -1 ;
        else enpassant_ = get_idx_from_notation(en_passant);

        castling_.setWe_00(castlings.find('K') != std::string::npos);
        castling_.setWe_000(castlings.find('Q') != std::string::npos);
        castling_.setEnemy_00(castlings.find('k') != std::string::npos);
        castling_.setEnemy_000(castlings.find('q') != std::string::npos);

        white_toact_ = who_to_move == "w" ? 1 : 0;

        if(white_toact_ != 1)
        {
            this->mirror();
            white_toact_ = 0;
        }

        check_castle_legality();

        this->set_zobrist();
}

std::vector<char> ChessBoard::_fill_printer(const int &print_mode, char spec_type) const
{
    std::vector<char> printer;

    std::vector<char> notations;
    if( print_mode == 0 )  notations =  std::vector<char> {'P', 'N', 'B', 'R', 'Q', 'K', 'p', 'n', 'b', 'r', 'q', 'k'};
    else if( print_mode == 1)  notations = std::vector<char> {'P', 'N', 'B', 'R', 'Q', 'K'};
    else if (print_mode == 2) notations =  std::vector<char> {'p', 'n', 'b', 'r', 'q', 'k'};
    else if (print_mode == 3) notations =  std::vector<char> {spec_type};

    for (int i = 0 ; i<64 ; ++i)
    {
        auto piece_pair = occupied_by(i);

        if(contains(piece_pair.first , notations))
            printer.emplace_back(piece_pair.first);

        else
             printer.emplace_back(',');
    }

    return printer;
}

void ChessBoard::print_to_console(const int& print_mode, char spec_type ) const
{
    auto printer = _fill_printer(print_mode, spec_type);
    int idx = 56;

    print("___A__B__C__D__E__F__G__H__");

    std::cout << int(idx/8) + 1;

    while( idx >= 0)
    {
        if ((idx + 1) % 8 == 0)
        {
            std::cout << " |" << printer.at(idx) << " |" << std::endl;
            idx -= 15;
            if (int(idx / 8) + 1 > 0)
            {
                std::cout << (int(idx / 8) + 1);
            }
        }
        else
        {
            std::cout << " |" << printer.at(idx);
            idx += 1;
        }
    }   
    print("___A__B__C__D__E__F__G__H__");
    print("\n");
}

void ChessBoard::print_bitboard(const uint64_t &bitboard) const
{
    ChessBoard cb;
    cb.pawns_ = bitboard;

    cb.print_to_console(3,'P');
}

