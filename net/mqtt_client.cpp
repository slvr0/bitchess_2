#include "mqtt_client.h"

MQTT_PIPE::MQTT_PIPE(const QHostAddress host_adress, const quint16 port, QObject* parent) :
    QMQTT::Client(host_adress, port, parent),
    qos_(0)

{
    QObject::connect(this, &QMQTT::Client::connected, this, &MQTT_PIPE::on_connect);
//    QObject::connect(this, &QMQTT::Client::received, this, &MQTT_PIPE::on_receive);
//    QObject::connect(this, &QMQTT::Client::subscribed, this, &MQTT_PIPE::on_subscribed);
}

void MQTT_PIPE::set_topic(const QString &topic)
{
    topic_ = topic;

    qDebug () << "client topic set to : " << topic_ << "";
}

void MQTT_PIPE::set_topic(std::string &topic)
{
    set_topic(QString(topic.c_str()));
}

QString MQTT_PIPE::get_topic() const
{
    return topic_;
}

bool MQTT_PIPE::get_is_connected() const
{
    return isConnectedToHost();
}

void MQTT_PIPE::on_connect()
{
    qDebug() << "client  connected!";

    subscribe(topic_, qos_);
}

void MQTT_PIPE::on_receive(const QMQTT::Message &message)
{
    std::string payload = QString::fromUtf8(message.payload()).toStdString();

    std::cout << "receive message: " << payload << std::endl;

//    publish_message(payload);
}

void MQTT_PIPE::on_subscribed(const QString &topic)
{
    qDebug() << "client subscribed to " << topic_;
}

QString MQTT_PIPE::get_publish_topic() const
{
    return publish_topic_;
}

void MQTT_PIPE::set_publish_topic(const QString &publish_topic)
{
    publish_topic_ = publish_topic;
}

void MQTT_PIPE::publish_message(const std::string message)
{
    publish_message(QString(message.c_str()));
}

void MQTT_PIPE::publish_message(const QString &message)
{
    QMQTT::Message qmqtt_msg;

    qmqtt_msg.setQos(qos_);
    qmqtt_msg.setTopic(publish_topic_);
    qmqtt_msg.setPayload(message.toUtf8());

//    qDebug() << "publishing...  to topic = " << publish_topic_ << " message = " << message << "\n";

    publish(qmqtt_msg);
}

MQTT_PIPE_THREAD::MQTT_PIPE_THREAD(QObject *parent) :
    QThread(parent)
{

}

void MQTT_PIPE_THREAD::append_and_allocate_client(MQTT_PIPE* client)
{
//    client->moveToThread(this);

    clients_.push_back(client);
}

std::vector<MQTT_PIPE *> MQTT_PIPE_THREAD::get_clients() const
{
    return clients_;
}



