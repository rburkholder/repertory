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
 * File:    config.hpp
 * Author:  raymond@burkholder.net
 * Project: Repertory/MQTT
 * Created: December 24, 2023 17:56:30
 */

#pragma once

#include <string>

namespace ou {
namespace mqtt {

struct Config {

  std::string sId;       // unique id for this instance
  std::string sHost;     // address or name listening on port 1833
  std::string sUserName; // username
  std::string sPassword; // password
  std::string sTopic;    // topic, should not have leading slash

  Config() {}

  Config(
    const std::string& sId_
  , const std::string& sHost_
  , const std::string& sUserName_
  , const std::string& sPassword_
  , const std::string& sTopic_
  )
  : sId( sId_ )
  , sHost( sHost_ )
  , sUserName( sUserName_ )
  , sPassword( sPassword_ )
  , sTopic( sTopic_ )
  {}

  Config(
    std::string&& sId_
  , std::string&& sHost_
  , std::string&& sUserName_
  , std::string&& sPassword_
  , std::string&& sTopic_
  )
  : sId( std::move( sId_ ) )
  , sHost( std::move( sHost_ ) )
  , sUserName( std::move( sUserName_ ) )
  , sPassword( std::move( sPassword_ ) )
  , sTopic( std::move( sTopic_ ) )
  {}

  Config( const Config& config )
  : sId( config.sId )
  , sHost( config.sHost )
  , sUserName( config.sUserName )
  , sPassword( config.sPassword )
  , sTopic( config.sTopic )
  {}

  const Config& operator=( const Config& config ) {
    sId = config.sId;
    sHost = config.sHost;
    sUserName = config.sUserName;
    sPassword = config.sPassword;
    sTopic = config.sTopic;
    return( *this );
  }

  const Config& operator=( Config&& config ) {
    sId = std::move( config.sId );
    sHost = std::move( config.sHost );
    sUserName = std::move( config.sUserName );
    sPassword = std::move( config.sPassword );
    sTopic = std::move( config.sTopic );
    return( *this );
  }

  Config( Config&& config )
  : sId( std::move( config.sId ) )
  , sHost( std::move( config.sHost ) )
  , sUserName( std::move( config.sUserName ) )
  , sPassword( std::move( config.sPassword ) )
  , sTopic( std::move( config.sTopic ) )
  {}
};

} // namespace mqtt
} // namespace ou
