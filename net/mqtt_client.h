#pragma once

#include <iostream>

#include <QObject>
#include <QHostAddress>
#include <QTextStream>
#include <QThread>

#include <qmqtt.h>

#include <vector>
#include "net/mqtt_client.h"

const QHostAddress default_host = QHostAddress::LocalHost;
const quint16 default_port = 1883;
const QString default_publish_topic = "mcts_nn_que";

class MQTT_PIPE;

class MQTT_PIPE_THREAD : public QThread
{
    Q_OBJECT

public :
    MQTT_PIPE_THREAD(QObject* parent = nullptr);

    void append_and_allocate_client(MQTT_PIPE *client);

    std::vector<MQTT_PIPE*> get_clients() const;

private:
    std::vector<MQTT_PIPE*> clients_;

};

class MQTT_PIPE : public QMQTT::Client
{
    Q_OBJECT
public:
    MQTT_PIPE(const QHostAddress host_adress = default_host, const quint16 port = default_port, QObject* parent = nullptr);

    virtual ~MQTT_PIPE(){}

    void set_topic(const QString& topic);
    void set_topic(std::string& topic);

    QString get_topic() const;
    bool get_is_connected() const;

    QString get_publish_topic() const;
    void set_publish_topic(const QString &publish_topic);

    void publish_message(const std::string message);
    void publish_message(const QString & message);

    void allocate_to_thread(QThread *thread);


private slots:
    void on_connect();
    void on_receive(const QMQTT::Message& message);
    void on_subscribed(const QString& topic);

private :
    QString topic_;
    QString publish_topic_;
    quint8 qos_;


};
