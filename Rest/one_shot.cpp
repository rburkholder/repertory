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
 * File:    one_shot.cpp
 * Project: Repertory/Telegram
 * Author:  raymond@burkholder.net
 * Created: February 17, 2023 10:02:25
 */

// TODO: look at http return codes

#include <boost/log/trivial.hpp>

#include "one_shot.hpp"

namespace ou {
namespace rest {

namespace { // anonymous

const static int nVersion( 11 );
const static std::string sUserAgent( "ounl.telegram/1.0" );

} // namespace anonymous

one_shot::one_shot(
  asio::any_io_executor ex,
  ssl::context& ssl_ctx
)
: ou::rest::handler( ex, ssl_ctx)
{
  //BOOST_LOG_TRIVIAL(info) << "telegram_bot::one_shot construction"; // ensuring proper timing of handling
}

one_shot::~one_shot() {
  //BOOST_LOG_TRIVIAL(info) << "telegram_bot::one_shot destruction";  // ensuring proper timing of handling
}

void one_shot::run(
  const std::string& sHost
, const std::string& sPort
, const std::string& sTarget
, int version
) {
  // Set SNI Hostname (many hosts need this to handshake successfully)
  if( !SSL_set_tlsext_host_name( m_stream.native_handle(), sHost.c_str() ) )
  {
    beast::error_code ec{ static_cast<int>( ::ERR_get_error()), asio::error::get_ssl_category() };
    BOOST_LOG_TRIVIAL(error) << ec.message();
    return;
  }

  // Set up an HTTP GET request message
  pRequestEmptyBody_t pRequest = std::make_shared<http::request<http::empty_body>>();
  auto& request( *pRequest );
  request.version( version );
  request.method( http::verb::get );
  request.set( http::field::host, sHost );
  //request_.set( http::field::user_agent, BOOST_BEAST_VERSION_STRING );
  request.set( http::field::user_agent, sUserAgent );

  request.target( sTarget );
  //req_.body() = json::serialize( jv );
  //req_.prepare_payload();

  // Look up the domain name
  m_resolver.async_resolve(
    sHost, sPort,
    beast::bind_front_handler(
      &one_shot::on_resolve,
      shared_from_this(),
      //[this](){ write_empty(); }
      std::bind( &one_shot::write_empty, shared_from_this(), std::move( pRequest ), std::placeholders::_1 ),
      //[this,p=std::move(pRequest)]( fDone_t&& fDone ){ write_empty( std::move( p ), std::move( fDone ) ); },
      []( bool, int, const std::string& ){} // prepopulated dummy entry
    )
  );
}

void one_shot::get(
  const std::string& sHost
, const std::string& sPort
, const std::string& sTarget
, fDone_t&& fDone
) {

  assert( fDone );

  // Set SNI Hostname (many hosts need this to handshake successfully)
  if( !SSL_set_tlsext_host_name( m_stream.native_handle(), sHost.c_str() ) )
  {
    beast::error_code ec{ static_cast<int>( ::ERR_get_error()), asio::error::get_ssl_category() };
    BOOST_LOG_TRIVIAL(error) << ec.message();
    fDone( false, ec.value(), ec.message() );
    return;
  }

  // Set up an HTTP GET request message
  pRequestEmptyBody_t pRequest = std::make_shared<http::request<http::empty_body>>();
  auto& request( *pRequest );
  request.version( nVersion );
  request.method( http::verb::get );
  request.set( http::field::host, sHost );
  request.set( http::field::user_agent, sUserAgent );

  //m_request_empty.target( sTarget );
  //const std::string s( "/bot" + sTelegramToken + "/" + sCommand );
  //BOOST_LOG_TRIVIAL(info) << "get request: '" << s << "'";
  request.target( sTarget );

  // Look up the domain name
  m_resolver.async_resolve(
    sHost, sPort,
    beast::bind_front_handler(
      &one_shot::on_resolve,
      shared_from_this(),
      //[this](){ write_empty(); }
      std::bind( &one_shot::write_empty, shared_from_this(), std::move( pRequest ), std::placeholders::_1 ),
      //[this,p=std::move(pRequest)]( fDone_t&& fDone ){ write_empty( std::move( p ), std::move( fDone ) ); },
      std::move( fDone )
    )
  );
}

void one_shot::get(
  const std::string& sHost
, const std::string& sPort
, const std::string& sTarget
, const std::string& sBody
, fDone_t&& fDone
) {

  assert( fDone );

  // Set SNI Hostname (many hosts need this to handshake successfully)
  if( !SSL_set_tlsext_host_name( m_stream.native_handle(), sHost.c_str() ) )
  {
    beast::error_code ec{ static_cast<int>( ::ERR_get_error()), asio::error::get_ssl_category() };
    BOOST_LOG_TRIVIAL(error) << ec.message();
    fDone( false, ec.value(), ec.message() );
    return;
  }

  // Set up an HTTP GET request message
  pRequestStringBody_t pRequest = std::make_shared<http::request<http::string_body>>();
  auto& request( *pRequest );
  request.version( nVersion );
  request.method( http::verb::get );
  request.set( http::field::host, sHost );
  request.set( http::field::user_agent, sUserAgent );
  request.set( http::field::content_type, "application/json" );

  //const std::string sTarget( "/bot" + sTelegramToken + "/" + sCommand );
  //BOOST_LOG_TRIVIAL(info) << "get request: '" << s << "'";
  request.target( sTarget );

  request.body() = sBody;
  request.prepare_payload();

  // Look up the domain name
  m_resolver.async_resolve(
    sHost, sPort,
    beast::bind_front_handler(
      &one_shot::on_resolve,
      shared_from_this(),
      //[this](){ write_body(); }
      std::bind( &one_shot::write_body, shared_from_this(), std::move( pRequest ), std::placeholders::_1 ),
      //[this,p=std::move(pRequest)]( fDone_t&& fDone ){ write_body( std::move( p ), std::move( fDone ) ); },
      std::move( fDone )
    )
  );
}

void one_shot::post(
  const std::string& sHost
, const std::string& sPort
, const std::string& sTarget
, const std::string& sBody
, fDone_t&& fDone
) {

  assert( fDone );

  // Set SNI Hostname (many hosts need this to handshake successfully)
  if( !SSL_set_tlsext_host_name( m_stream.native_handle(), sHost.c_str() ) )
  {
    beast::error_code ec{ static_cast<int>( ::ERR_get_error()), asio::error::get_ssl_category() };
    BOOST_LOG_TRIVIAL(error) << ec.message();
    fDone( false, ec.value(), ec.message() );
    return;
  }

  // Set up an HTTP GET request message
  pRequestStringBody_t pRequest = std::make_shared<http::request<http::string_body>>();
  auto& request( *pRequest );
  request.version( nVersion );
  request.method( http::verb::post );
  request.set( http::field::host, sHost );
  request.set( http::field::user_agent, sUserAgent );
  request.set( http::field::content_type, "application/json" );

  //const std::string sTarget( "/bot" + sTelegramToken + "/" + sCommand );
  //BOOST_LOG_TRIVIAL(info) << "post target: '" << sTarget << "'";
  request.target( sTarget );

  request.body() = sBody;
  request.prepare_payload();

  //BOOST_LOG_TRIVIAL(info) << m_request_body;

  // Look up the domain name
  m_resolver.async_resolve(
    sHost, sPort,
    beast::bind_front_handler(
      &one_shot::on_resolve,
      shared_from_this(),
      //[this](){ write_body(); }
      std::bind( &one_shot::write_body, shared_from_this(), std::move( pRequest ), std::placeholders::_1 ),
      //[this,p=std::move(pRequest)]( fDone_t&& fDone ){ write_body( std::move( p ), std::move( fDone ) ); },
      std::move( fDone )
    )
  );
}

void one_shot::delete_(
  const std::string& sHost
, const std::string& sPort
, const std::string& sTarget
, fDone_t&& fDone
) {

  assert( fDone );

  // Set SNI Hostname (many hosts need this to handshake successfully)
  if( !SSL_set_tlsext_host_name( m_stream.native_handle(), sHost.c_str() ) )
  {
    beast::error_code ec{ static_cast<int>( ::ERR_get_error()), asio::error::get_ssl_category() };
    BOOST_LOG_TRIVIAL(error) << ec.message();
    fDone( false, ec.value(), ec.message() );
    return;
  }

  // Set up an HTTP GET request message
  pRequestEmptyBody_t pRequest = std::make_shared<http::request<http::empty_body>>();
  auto& request( *pRequest );
  request.version( nVersion );
  request.method( http::verb::delete_ );
  request.set( http::field::host, sHost );
  //request_.set( http::field::user_agent, BOOST_BEAST_VERSION_STRING );
  request.set( http::field::user_agent, sUserAgent );

  request.target( sTarget );

  // Look up the domain name
  m_resolver.async_resolve(
    sHost, sPort,
    beast::bind_front_handler(
      &one_shot::on_resolve,
      shared_from_this(),
      //[this](){ write_empty(); }
      std::bind( &one_shot::write_empty, shared_from_this(), std::move( pRequest ), std::placeholders::_1 ),
      //[this,p=std::move(pRequest)]( fDone_t&& fDone ){ write_empty( std::move( p ), std::move( fDone ) ); },
      std::move( fDone )
    )
  );
}

} // namespace rest
} // namespace ou
