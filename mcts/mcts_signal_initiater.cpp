#include "mcts_signal_initiater.h"

#include <QString>
#include "autoplay/board_wrapper.h"
#include "mcts/mcts_search.h"

#include <iostream>
#include <string>
#include <cassert>
#include "core/chess_move.h"
#include "utils/global_utils.cpp"

MCTSSignalInitiater::MCTSSignalInitiater(std::vector<Subscriber*> subscribers, Publisher* mqtt_publisher, QObject* parent) :
    tree_search_(nullptr),
    cached_positions_(std::make_unique<NetCachedPositions> ()),
    mqtt_publisher_(mqtt_publisher),
    QObject(parent)
{
    for(auto & mqtt_subscriber : subscribers)
    {
        //print(mqtt_subscriber->topic());

        if(QString::compare(mqtt_subscriber->topic(), QString("mcts_tree_init"),Qt::CaseInsensitive) == 0)
                connect(mqtt_subscriber, &Subscriber::received, this, &MCTSSignalInitiater::query_mcts);
        else if(QString::compare(mqtt_subscriber->topic(), QString("mcts_cache_position"),Qt::CaseInsensitive) == 0)
                connect(mqtt_subscriber, &Subscriber::received, this, &MCTSSignalInitiater::query_position);
    }

    int max_entries = 1000;
    float rollout_entries_ratio = 0.6f;
    int n_rollouts = int(max_entries * rollout_entries_ratio);

    tree_search_ = std::make_unique<mcts::TreeSearch>(&move_gen_, 1, max_entries, n_rollouts, cached_positions_.get());

    print("signals mcts_tree_init and mcts_cache_position connected, tree search is set, cached positions is initiated");
}

void MCTSSignalInitiater::clear_tree()
{
      tree_search_->clear_tree();
}

void MCTSSignalInitiater::query_mcts(const QMQTT::Message &message)
{
    std::string decoded = QString::fromUtf8(message.payload()).toStdString();

    bool ret = this->handshake_message(decoded);

    if(ret)
    {
        if(!is_search_ongoing_)
        {
            init_message_ = message;
            tree_search_->init_tree(ChessBoard(decoded));
            is_search_ongoing_ = true;
        }

        ChessBoard que_this =   tree_search_->start_search();

        //send chessboard fen to python... , query position will return and get going again

    }
}

void MCTSSignalInitiater::query_position(const QMQTT::Message &message)
{
     print("query position!");

     std::string decoded = QString::fromUtf8(message.payload()).toStdString();

     bool ret = this->handshake_message(decoded);

     //message will be a fen string of position, then mapped nn idcs with logits return.

     if(ret)
     {
         ChessBoard cb {decoded};
         int z_key = cb.get_zobrist();

         auto cached_position_data = decode_query_position(decoded);

         cached_positions_->add(cached_position_data.first, cached_position_data.second);

        if(is_search_ongoing_)
        {
            query_mcts(init_message_);
        }

         //print(cached_positions_->entries());

     }
}

bool MCTSSignalInitiater::handshake_message(const std::string &message)
{
    //check for jibbersh that we cannot create a tree from
    std::string error_message ="failed to initiate chess position from mqtt message, aborting search\n";

    bool ret = true;

    print("handshaking message... this wont throw error atm, gotta find a way to check this");

    try
    {
        ChessBoard cb  {message};
    }
    catch (...)
    {
        std::cout << error_message;
        ret = false;
        return ret;
    }
    return ret;
}

std::pair<uint64_t, std::vector<std::pair<int, float>>> MCTSSignalInitiater::decode_query_position(std::string decoded) const
{
    std::pair<uint64_t, std::vector<std::pair<int, float>>> decoded_position_data;

    std::string fen;

    std::vector<int> nn_idcs;
    std::vector<float> nn_logits;

    //first fen, separated by {
    size_t fen_separator = decoded.find("{");
    size_t message_end = decoded.find("}");

    fen = decoded.substr(0, fen_separator);

    ChessBoard cb{fen};
    decoded_position_data.first = cb.get_zobrist();

    std::vector<int> val_separators = find_all_char_positions_in_string(decoded, ':');

    bool valid_logs = true;

    if(val_separators.size() == 0) valid_logs = false;

    if(valid_logs)
    {
        std::vector<std::string> substrings;

        substrings.emplace_back(decoded.substr(fen_separator + 1, val_separators.at(0) - fen_separator - 1));

        for(int i  = 0 ; i < val_separators.size() -1 ; ++i)
        {
            substrings.emplace_back(decoded.substr(val_separators.at(i)+ 1, val_separators.at(i+1) - val_separators.at(i) - 1));
        }
        substrings.emplace_back(decoded.substr(val_separators.at(val_separators.size() - 1) + 1 , message_end - val_separators.at(val_separators.size() - 1) - 1  ));

        for(int i = 0 ; i < substrings.size(); i+=2)
        {
            nn_idcs.emplace_back(std::stoi(substrings.at(i)));
            nn_logits.emplace_back((QString(substrings.at(i + 1).c_str()).toFloat()));

            int nn = std::stoi(substrings.at(i));
            float log = QString(substrings.at(i + 1).c_str()).toFloat();

            decoded_position_data.second.emplace_back(std::pair<int, float>
            (nn, log)   );
        }

        for(int i = 0 ; i < nn_idcs.size() ; ++i)
        {
            std::cout << nn_idcs.at(i) << " : " << nn_logits.at(i) << std::endl;
        }
    }

    return decoded_position_data;
}
