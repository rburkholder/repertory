/************************************************************************
 * Copyright(c) 2023, One Unified. All rights reserved.                 *
 * email: info@oneunified.net                                           *
 *                                                                      *
 * This file is provided as is WITHOUT ANY WARRANTY                     *
 *  without even the implied warranty of                                *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                *
 *                                                                      *
 * This software may not be used nor distributed without proper license *
 * agreement.                                                           *
 *                                                                      *
 * See the file LICENSE.txt for redistribution information.             *
 ************************************************************************/

/*
 * File:    mqtt.hpp
 * Project: Repertory/MQTT
 * Author:  raymond@burkholder.net
 * Created: December 24, 2023 17:56:30
 */

// 2024/01/20 maybe replace with https://github.com/mireo/async-mqtt5
//   but rabbitmq supports mqtt 3.1.1 only

#pragma once

#include <mutex>
#include <string>
#include <thread>
#include <stdexcept>
#include <functional>
#include <string_view>
#include <unordered_map>

#include <MQTTClient.h>

#include "config.hpp"

namespace ou {

class Mqtt {
public:

  struct runtime_error: std::runtime_error {
    int rc;
    runtime_error( const std::string& e, int rc_ )
    : std::runtime_error( e ) {}
  };

  Mqtt( const mqtt::Config& );
  Mqtt( mqtt::Config&& );
  ~Mqtt();

  using fPublishComplete_t = std::function<void(bool,int)>;
  void Publish( const std::string& sTopic, const std::string& sMessage, fPublishComplete_t&& );

  // send and forget, errors are simply logged
  using fMessage_t = std::function<void( const std::string_view& svTopic, const std::string_view& svMessage )>;
  void Subscribe( const std::string_view& svTopic, fMessage_t&& );
  void UnSubscribe( const std::string_view& svTopic );

protected:
private:

  enum class EState{ init, created, connecting, connected, start_reconnect, retry_connect, disconnecting, destruct } m_state;

  mqtt::Config m_config;

  std::thread m_threadConnect;

  MQTTClient m_clientMqtt;
  MQTTClient_connectOptions m_conn_opts;
  MQTTClient_message m_pubmsg;

  std::mutex m_mutexDeliveryToken;

  using umapDeliveryToken_t = std::unordered_map<MQTTClient_deliveryToken, fPublishComplete_t>;
  umapDeliveryToken_t m_umapDeliveryToken;

  fMessage_t m_fMessage;

  void Init();

  static int MessageArrived( void* context, char* topicName, int topicLen, MQTTClient_message* message );
  static void DeliveryComplete( void* context, MQTTClient_deliveryToken token );
  static void ConnectionLost( void* context, char* cause );

  void Connect();

};

} // namespace ou
