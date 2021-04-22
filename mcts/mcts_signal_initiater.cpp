#include "mcts_signal_initiater.h"

#include <QString>
#include "autoplay/board_wrapper.h"
#include "mcts/mcts_search.h"

#include <iostream>
#include <string>
#include <cassert>
#include "core/chess_move.h"
#include "utils/global_utils.cpp"


MCTSSignalInitiater::MCTSSignalInitiater(MQTT_PIPE_THREAD& comm_thread, QObject* parent) :
    tree_search_(nullptr),
    cached_positions_(std::make_unique<NetCachedPositions> ()),
    QObject(parent)
{
    int max_entries =  10000;
    float rollout_entries_ratio = 1.5f;
    int n_rollouts = int(max_entries * rollout_entries_ratio);

    cached_positions_ = std::make_unique<NetCachedPositions> ();

    for(const auto & mqtt_pipe : comm_thread.get_clients())
    {     
        if(QString::compare(mqtt_pipe->get_topic(), QString("mcts_tree_init"),Qt::CaseInsensitive) == 0)
        {
                connect(mqtt_pipe, &MQTT_PIPE::received, this, &MCTSSignalInitiater::query_mcts);
                init_finish_pipe_ = mqtt_pipe;
        }
        else if(QString::compare(mqtt_pipe->get_topic(), QString("mcts_cache_position"),Qt::CaseInsensitive) == 0)
        {
                connect(mqtt_pipe, &MQTT_PIPE::received, this, &MCTSSignalInitiater::query_position);
                cache_query_pipe_ = mqtt_pipe;
        }
    }

    if(!cache_query_pipe_) throw;

    tree_search_ = std::make_unique<mcts::TreeSearch>(&move_gen_, 1, max_entries, n_rollouts, cached_positions_.get(), cache_query_pipe_, init_finish_pipe_);

    print("MCTS Signal Tree Connected");
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
        if(!tree_search_->get_is_init())
        {
            init_message_ = message;
            tree_search_->init_tree(ChessBoard(decoded));
        }

        tree_search_->start_search();
    }
}

void MCTSSignalInitiater::query_position(const QMQTT::Message &message)
{
    std::string decoded = QString::fromUtf8(message.payload()).toStdString();

    bool ret = this->handshake_message(decoded);

    auto cached_position_data = decode_query_position(decoded);

    cached_positions_->add(cached_position_data.first, cached_position_data.second);

    if(tree_search_->get_is_init())
    {
        query_mcts(init_message_);
    }
}

bool MCTSSignalInitiater::handshake_message(const std::string &message)
{
    //check for jibbersh that we cannot create a tree from
    std::string error_message ="failed to initiate chess position from mqtt message, aborting search\n";

    bool ret = true;

//    print("handshaking message... this wont throw error atm, gotta find a way to check this");

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

std::pair<uint64_t, std::map<int, float>> MCTSSignalInitiater::decode_query_position(std::string decoded) const
{
    std::pair<uint64_t, std::map<int, float>> decoded_position_data;

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

            decoded_position_data.second[nn] = log;
        }
    }

    return decoded_position_data;
}
