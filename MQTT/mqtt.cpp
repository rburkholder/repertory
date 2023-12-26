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
  File:    mqtt.cpp
  Project: Repertory/MQTT
  Author:  raymond@burkholder.net
  Created: December 24, 2023 17:56:30
  copied from, and need to backport to nut2mqtt
*/

#include <chrono>
#include <cassert>
#include <iostream>

#include "mqtt.hpp"

// documentation: https://eclipse.github.io/paho.mqtt.c/MQTTClient/html/_m_q_t_t_client_8h.html

namespace {
  unsigned int c_nQOS( 1 );
  unsigned int c_nTimeOut( 2 ); // seconds
}

namespace ou {
namespace mqtt {

Mqtt::Mqtt( const Config& choices )
: m_state( EState::init )
, m_conn_opts( MQTTClient_connectOptions_initializer )
, m_pubmsg( MQTTClient_message_initializer )
, m_sMqttUrl( "tcp://" + choices.sHost + ":1883" )
, m_fMessage( nullptr )
{
  m_conn_opts.keepAliveInterval = 20;
  m_conn_opts.cleansession = 1;
  m_conn_opts.connectTimeout = c_nTimeOut;
  m_conn_opts.username = choices.sUserName.c_str();
  m_conn_opts.password = choices.sPassword.c_str();

  int rc;

  rc = MQTTClient_create(
    &m_clientMqtt, m_sMqttUrl.c_str(), choices.sId.c_str(),
    MQTTCLIENT_PERSISTENCE_NONE, nullptr
    );

  if ( MQTTCLIENT_SUCCESS != rc ) {
    throw( runtime_error( "Failed to create client", rc ) );
  }

  m_state = EState::created;

  rc = MQTTClient_setCallbacks( m_clientMqtt, this, &Mqtt::ConnectionLost, &Mqtt::MessageArrived, &Mqtt::DeliveryComplete );

  try {
    rc = MQTTClient_connect( m_clientMqtt, &m_conn_opts );
  }
  catch (...) {
    std::cerr << "mqtt initial connect broken" << std::endl;
  }

  if ( MQTTCLIENT_SUCCESS == rc ) {
    m_state = EState::connected;
  }
  else {
    m_state = EState::connecting;
    Connect();
  }

}

Mqtt::~Mqtt() {
  int rc {};
  switch ( m_state ) {
    case EState::retry_connect:
      m_state = EState::disconnecting;
      if ( m_threadConnect.joinable() ) m_threadConnect.join();
      // fall through to EState::connected:
    case EState::connected:
      m_state = EState::disconnecting;
      rc = MQTTClient_disconnect( m_clientMqtt, 1000 );
      if ( MQTTCLIENT_SUCCESS != rc ) {
        std::cerr << "Failed to disconnect, return code " << rc << std::endl;
      }
      m_state = EState::destruct;
      break;
    default:
      break;
  }

  if ( EState::init != m_state ) {
    assert( EState::destruct == m_state );
    MQTTClient_destroy( &m_clientMqtt );
  }
}

void Mqtt::Connect() {
  assert( EState::init != m_state );
  if ( 1 == MQTTClient_isConnected( m_clientMqtt ) ) {
    assert( EState::connected == m_state );
    std::cerr << "mqtt is already connected" << std::endl;
  }
  else {
    m_state = EState::retry_connect;
    m_threadConnect = std::move( std::thread(
      [this](){
        while ( EState::retry_connect == m_state ) {
          try {
            int rc = MQTTClient_connect( m_clientMqtt, &m_conn_opts );
            if ( MQTTCLIENT_SUCCESS == rc ) {
              m_state = EState::connected;
              std::cout << "mqtt re-connected" << std::endl;
            }
            else {
              //std::cerr << "mqtt reconnect wait" << std::endl;
              std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );
            }
          }
          catch (...) {
            std::cerr << "mqtt retry reconnect broken" << std::endl;
          }
        }
      } ) );
  }
}

void Mqtt::Publish( const std::string& sTopic, const std::string& sMessage, fPublishComplete_t&& fPublishComplete ) {
  if ( EState::connected == m_state ) {
    m_pubmsg.payload = (void*) sMessage.begin().base();
    m_pubmsg.payloadlen = sMessage.size();
    m_pubmsg.qos = c_nQOS;
    m_pubmsg.retained = 0;
    MQTTClient_deliveryToken token;
    int rc = MQTTClient_publishMessage( m_clientMqtt, sTopic.c_str(), &m_pubmsg, &token );
    if ( MQTTCLIENT_SUCCESS != rc ) {
      fPublishComplete( false, rc );
      //throw( runtime_error( "Failed to publish message", rc ) );
    }
    else {
      std::lock_guard<std::mutex> lock( m_mutexDeliveryToken );
      umapDeliveryToken_t::iterator iterDeliveryToken = m_umapDeliveryToken.find( token );
      if ( m_umapDeliveryToken.end() == iterDeliveryToken ) {
        m_umapDeliveryToken.emplace( token, std::move( fPublishComplete ) );
      }
      else {
        std::cerr << "delivery token " << token << " already delivered" << std::endl;
        fPublishComplete( true, 0 );
        assert( nullptr == iterDeliveryToken->second );
        m_umapDeliveryToken.erase( iterDeliveryToken );
      }
    }
  }
}

void Mqtt::Subscribe( const std::string_view& topic, fMessage_t&& fMessage ) {
  m_fMessage = std::move( fMessage );
  assert( m_clientMqtt );
  // TODO: memory leaks on topic?
  int result = MQTTClient_subscribe( m_clientMqtt, topic.begin(), 1 );
  assert( MQTTCLIENT_SUCCESS == result );
}

void Mqtt::UnSubscribe( const std::string_view& topic ) {
  assert( m_clientMqtt );
  // TODO: memory leaks on topic?
  int result = MQTTClient_unsubscribe( m_clientMqtt, topic.begin() );
  assert( MQTTCLIENT_SUCCESS == result );
  m_fMessage = nullptr;
}

void Mqtt::ConnectionLost( void* context, char* cause ) {
  assert( context );
  Mqtt* self = reinterpret_cast<Mqtt*>( context );
  std::cerr << "mqtt connection lost, reconnecting ..." << std::endl;
  assert( EState::connected == self->m_state );
  self->m_state = EState::start_reconnect;
  self->Connect();
  //std::cout << "mqtt started reconnect" << std::endl;
}

int Mqtt::MessageArrived( void* context, char* topicName, int topicLen, MQTTClient_message* message ) {
  assert( context );
  Mqtt* self = reinterpret_cast<Mqtt*>( context );
  assert( 0 == topicLen ); // for some reason in comes in this way
  //std::cout << "mqtt message: " << std::string( topicName ) << " " << std::string( (const char*) message->payload, message->payloadlen ) << std::endl;
  const std::string_view svTopic( topicName );
  const std::string_view svMessage( (char*)message->payload, message->payloadlen );
  if ( self->m_fMessage ) self->m_fMessage( svTopic, svMessage );
  MQTTClient_freeMessage( &message );
  MQTTClient_free( topicName );
  return 1;
}

void Mqtt::DeliveryComplete( void* context, MQTTClient_deliveryToken token ) {
	// not called with QoS0
  assert( context );
  Mqtt* self = reinterpret_cast<Mqtt*>( context );
  //std::cout << "mqtt delivery complete" << std::endl;
  std::lock_guard<std::mutex> lock( self->m_mutexDeliveryToken );
  umapDeliveryToken_t::iterator iterDeliveryToken = self->m_umapDeliveryToken.find( token );
  if ( self->m_umapDeliveryToken.end() == iterDeliveryToken ) {
    std::cerr << "delivery token " << token << " not yet registered" << std::endl;
    self->m_umapDeliveryToken.emplace( token, nullptr );
  }
  else {
    iterDeliveryToken->second( true, 0 );
    self->m_umapDeliveryToken.erase( iterDeliveryToken );
  }
}

} // namespace mqtt
} // namespace ou
