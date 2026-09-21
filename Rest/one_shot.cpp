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

  m_fDone = []( bool, int, const std::string& ){}; // prepopulated dummy entry

  // Set up an HTTP GET request message
  pRequestEmptyBody_t pRequest = std::make_shared<http::request<http::empty_body>>();
  pRequest->version( version );
  pRequest->method( http::verb::get );
  pRequest->set( http::field::host, sHost );
  //request_.set( http::field::user_agent, BOOST_BEAST_VERSION_STRING );
  pRequest->set( http::field::user_agent, sUserAgent );

  pRequest->target( sTarget );
  //req_.body() = json::serialize( jv );
  //req_.prepare_payload();

  m_pRequestBody = std::move( pRequest );

  // Look up the domain name
  m_resolver.async_resolve(
    sHost, sPort,
    beast::bind_front_handler(
      &one_shot::on_resolve,
      shared_from_this()
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

  m_fDone = std::move( fDone );

  // Set up an HTTP GET request message
  pRequestEmptyBody_t pRequest = std::make_shared<http::request<http::empty_body>>();
  //auto& request( *pRequest );
  pRequest->version( nVersion );
  pRequest->method( http::verb::get );
  pRequest->set( http::field::host, sHost );
  pRequest->set( http::field::user_agent, sUserAgent );

  //m_request_empty.target( sTarget );
  //const std::string s( "/bot" + sTelegramToken + "/" + sCommand );
  //BOOST_LOG_TRIVIAL(info) << "get request: '" << sTarget << "'";
  pRequest->target( sTarget );

  m_pRequestBody = std::move( pRequest );

  // Look up the domain name
  m_resolver.async_resolve(
    sHost, sPort,
    beast::bind_front_handler(
      &one_shot::on_resolve,
      shared_from_this()
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

  m_fDone = std::move( fDone );

  // Set up an HTTP GET request message
  pRequestStringBody_t pRequest = std::make_shared<http::request<http::string_body>>();
  //auto& request( *pRequest );
  pRequest->version( nVersion );
  pRequest->method( http::verb::get );
  pRequest->set( http::field::host, sHost );
  pRequest->set( http::field::user_agent, sUserAgent );
  pRequest->set( http::field::content_type, "application/json" );

  //const std::string sTarget( "/bot" + sTelegramToken + "/" + sCommand );
  //BOOST_LOG_TRIVIAL(info) << "get request: '" << sTarget << "', '" << sBody << "'";
  pRequest->target( sTarget );

  pRequest->body() = sBody;
  pRequest->prepare_payload();

  m_pRequestBody = std::move( pRequest );

  // Look up the domain name
  m_resolver.async_resolve(
    sHost, sPort,
    beast::bind_front_handler(
      &one_shot::on_resolve,
      shared_from_this()
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

  m_fDone = std::move( fDone );

  // Set up an HTTP GET request message
  pRequestStringBody_t pRequest = std::make_shared<http::request<http::string_body>>();
  pRequest->version( nVersion );
  pRequest->method( http::verb::post );
  pRequest->set( http::field::host, sHost );
  pRequest->set( http::field::user_agent, sUserAgent );
  pRequest->set( http::field::content_type, "application/json" );

  //const std::string sTarget( "/bot" + sTelegramToken + "/" + sCommand );
  //BOOST_LOG_TRIVIAL(info) << "post target: '" << sTarget << "', '" << sBody << "'";
  pRequest->target( sTarget );

  pRequest->body() = sBody;
  pRequest->prepare_payload();

  m_pRequestBody = std::move( pRequest );

  //BOOST_LOG_TRIVIAL(info) << m_request_body;

  // Look up the domain name
  m_resolver.async_resolve(
    sHost, sPort,
    beast::bind_front_handler(
      &one_shot::on_resolve,
      shared_from_this()
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

  m_fDone = std::move( fDone );

  // Set up an HTTP GET request message
  pRequestEmptyBody_t pRequest = std::make_shared<http::request<http::empty_body>>();
  pRequest->version( nVersion );
  pRequest->method( http::verb::delete_ );
  pRequest->set( http::field::host, sHost );
  //request_.set( http::field::user_agent, BOOST_BEAST_VERSION_STRING );
  pRequest->set( http::field::user_agent, sUserAgent );

  pRequest->target( sTarget );

  m_pRequestBody = std::move( pRequest );

  // Look up the domain name
  m_resolver.async_resolve(
    sHost, sPort,
    beast::bind_front_handler(
      &one_shot::on_resolve,
      shared_from_this()
    )
  );
}

} // namespace rest
} // namespace ou
