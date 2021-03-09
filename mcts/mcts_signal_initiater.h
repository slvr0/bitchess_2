#pragma once
#include <memory>
#include <iostream>

#include <QObject>

#include <qmqtt.h>

#include "core/chess_board.h"

#include <qmqtt_client.h>
#include <qmqtt_frame.h>
#include <qmqtt_global.h>
#include <qmqtt_message.h>

#include <qt5/QtNetwork/QHostAddress>
#include <qt5/QtCore/QObject>
#include <qt5/QtWidgets/QWidget>
#include <qt5/QtCore/QThread>

#include <QCoreApplication>
#include <QTimer>
#include <iostream>
#include <net/mqtt_client.h>
#include "core/move_generator.h"
#include "mcts/cached_positions.h"
#include "mcts/mcts_search.h"

class MCTSSignalInitiater : public QObject
{
    Q_OBJECT
public:
    explicit MCTSSignalInitiater(MQTT_PIPE_THREAD& comm_thread, QObject* parent = NULL);
    virtual ~MCTSSignalInitiater() {}

    void clear_tree();

public  slots :
    void query_mcts(const QMQTT::Message& message);
    void query_position(const QMQTT::Message& message);

private :
    bool handshake_message(const std::string &message);

    std::pair<uint64_t, std::map<int, float> > decode_query_position(std::string decoded) const;

private:
    std::unique_ptr<NetCachedPositions> cached_positions_;

    MQTT_PIPE* init_finish_pipe_ = nullptr;
    MQTT_PIPE* cache_query_pipe_= nullptr;

    MoveGenerator move_gen_;

    QMQTT::Message  init_message_;
    bool is_search_ongoing_ = false;

    std::unique_ptr<mcts::TreeSearch> tree_search_;

    int max_entries_;
    int n_rollouts_;


};

