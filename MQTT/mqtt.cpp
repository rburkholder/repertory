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

Mqtt::Mqtt( const mqtt::Config& choices )
: m_state( EState::init )
, m_config( choices )
, m_fMessage( nullptr )
{
  Init( choices.sId );
}

Mqtt::Mqtt( const mqtt::Config& choices, const std::string& sId )
: m_state( EState::init )
, m_config( choices )
, m_fMessage( nullptr )
{
  Init( sId );
}

Mqtt::Mqtt( mqtt::Config&& choices )
: m_state( EState::init )
, m_config( std::move( choices ) )
, m_fMessage( nullptr )
{
  Init( choices.sId );
}

void Mqtt::Init( const std::string& sId ) {

  const std::string sMqttUrl("tcp://" + m_config.sHost + ':' + m_config.sPort );

  MQTTClient_connectOptions options = MQTTClient_connectOptions_initializer;
  options.keepAliveInterval = 20;
  options.cleansession = 1;
  options.reliable = 0;
  options.connectTimeout = c_nTimeOut;
  options.username = m_config.sUserName.c_str();
  options.password = m_config.sPassword.c_str();

  int result;

  result = MQTTClient_create(
    &m_clientMqtt, sMqttUrl.c_str(), sId.c_str(),
    MQTTCLIENT_PERSISTENCE_NONE, nullptr
    );

  //std::cout << "ou::mqtt create status " << result << std::endl;

  if ( MQTTCLIENT_SUCCESS != result ) {
    throw( runtime_error( "Failed to create client", result ) );
  }

  m_state = EState::created;

  result = MQTTClient_setCallbacks( m_clientMqtt, this, &Mqtt::ConnectionLost, &Mqtt::MessageArrived, &Mqtt::DeliveryComplete );
  assert( MQTTCLIENT_SUCCESS == result ); // MQTTCLIENT_FAILURE  on error

  try {
    result = MQTTClient_connect( m_clientMqtt, &options );
  }
  catch (...) {
    std::cerr << "mqtt initial connect broken" << std::endl;
  }

  //std::cout << "ou::mqtt connect status " << result << std::endl;

  if ( MQTTCLIENT_SUCCESS == result ) {
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

            MQTTClient_connectOptions options = MQTTClient_connectOptions_initializer;
            options.keepAliveInterval = 20;
            options.cleansession = 1;
            options.reliable = 0;
            options.connectTimeout = c_nTimeOut;
            options.username = m_config.sUserName.c_str();
            options.password = m_config.sPassword.c_str();

            int result = MQTTClient_connect( m_clientMqtt, &options );
            if ( MQTTCLIENT_SUCCESS == result ) {
              m_state = EState::connected;
              std::cout << "mqtt re-connected" << std::endl;
            }
            else {
              std::cerr << "mqtt reconnect wait" << std::endl;
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
  const std::string_view svTopic( sTopic );
  const std::string_view svMessage( sMessage );
  Publish( svTopic, svMessage, std::move( fPublishComplete ) );
}

void Mqtt::Publish( const std::string_view& svTopic, const std::string_view& svMessage, fPublishComplete_t&& fPublishComplete ) {

  if ( EState::connected == m_state ) {
    MQTTClient_deliveryToken token;

    int result = MQTTClient_publish( m_clientMqtt, svTopic.begin(), svMessage.size(), svMessage.begin(), c_nQOS, 0, &token );

    if ( MQTTCLIENT_SUCCESS != result ) {
      fPublishComplete( false, result );
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
  assert( EState::connected == m_state );
  // TODO: memory leaks on topic?
  int result = MQTTClient_subscribe( m_clientMqtt, topic.begin(), c_nQOS );
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

} // namespace ou
