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
 * File:    one_shot.hpp
 * Project: Repertory/Telegram
 * Author:  raymond@burkholder.net
 * Created: February 17, 2023 10:02:25
 */

// TODO: look at http return codes

#include <boost/log/trivial.hpp>

#include "one_shot.hpp"

namespace ou {
namespace telegram {
namespace bot {
namespace session {

namespace { // anonymous

const static int nVersion( 11 );
const static std::string sUserAgent( "ounl.telegram/1.0" );

// Report a failure
void fail( beast::error_code ec, char const* what ) {
  BOOST_LOG_TRIVIAL(error) << what << ": (" << ec.value() << ") " << ec.message();
}

} // namespace anonymous

one_shot::one_shot(
  asio::any_io_executor ex,
  ssl::context& ssl_ctx
)
: m_resolver( ex )
, m_stream( ex, ssl_ctx )
{
  //BOOST_LOG_TRIVIAL(info) << "telegram_bot::one_shot construction"; // ensuring proper timing of handling
}

one_shot::~one_shot() {
  //BOOST_LOG_TRIVIAL(info) << "telegram_bot::one_shot destruction";  // ensuring proper timing of handling
  //m_stream.shutdown();  // doesn't like this
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
  auto request( *pRequest );
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
      []( bool, int, const std::string& ){} // prepopulated dummy entry
    )
  );
}

void one_shot::get(
  const std::string& sHost
, const std::string& sPort
, const std::string& sTelegramToken
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
  auto request( *pRequest );
  request.version( nVersion );
  request.method( http::verb::get );
  request.set( http::field::host, sHost );
  request.set( http::field::user_agent, sUserAgent );

  //m_request_empty.target( sTarget );
  const std::string s( "/bot" + sTelegramToken + "/" + sTarget );
  //BOOST_LOG_TRIVIAL(info) << "get request: '" << s << "'";
  request.target( s );

  // Look up the domain name
  m_resolver.async_resolve(
    sHost, sPort,
    beast::bind_front_handler(
      &one_shot::on_resolve,
      shared_from_this(),
      //[this](){ write_empty(); }
      std::bind( &one_shot::write_empty, shared_from_this(), std::move( pRequest ), std::placeholders::_1 ),
      std::move( fDone )
    )
  );
}

void one_shot::get(
  const std::string& sHost
, const std::string& sPort
, const std::string& sTelegramToken
, const std::string& sCommand
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
  auto request( *pRequest );
  request.version( nVersion );
  request.method( http::verb::get );
  request.set( http::field::host, sHost );
  request.set( http::field::user_agent, sUserAgent );
  request.set( http::field::content_type, "application/json" );

  const std::string sTarget( "/bot" + sTelegramToken + "/" + sCommand );
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
      std::move( fDone )
    )
  );
}

void one_shot::post(
  const std::string& sHost
, const std::string& sPort
, const std::string& sTelegramToken
, const std::string& sCommand
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
  auto request( *pRequest );
  request.version( nVersion );
  request.method( http::verb::post );
  request.set( http::field::host, sHost );
  request.set( http::field::user_agent, sUserAgent );
  request.set( http::field::content_type, "application/json" );

  const std::string sTarget( "/bot" + sTelegramToken + "/" + sCommand );
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
  auto request( *pRequest );
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
      std::move( fDone )
    )
  );
}

// ===== private

void one_shot::on_resolve(

  fWriteRequest_t&& fWrite,
  fDone_t&& fDone,
  beast::error_code ec,
  tcp::resolver::results_type results
) {
  if ( ec ) {
    fail( ec, "os.on_resolve");
    fDone( false, ec.value(), "os.on_resolve" );
  }
  else {
    // Set a timeout on the operation
    beast::get_lowest_layer( m_stream ).expires_after( std::chrono::seconds( 15 ) );

    //BOOST_LOG_TRIVIAL(info) << "os.on_resolve";

    // Make the connection on the IP address we get from a lookup
    beast::get_lowest_layer( m_stream )
      .async_connect(
        results,
        beast::bind_front_handler(
          &one_shot::on_connect,
          shared_from_this(),
          std::move( fWrite ),
          std::move( fDone )
        )
      );
  }
}

void one_shot::on_connect(
  fWriteRequest_t&& fWrite, fDone_t&& fDone,
  beast::error_code ec, tcp::resolver::results_type::endpoint_type et
) {
  if ( ec ) {
    fail( ec, "os.on_connect" );
    fDone( false, ec.value(), "os.on_connect" );
  }
  else {

    //BOOST_LOG_TRIVIAL(info) << "os.on_connect";

    // Perform the SSL handshake
    m_stream.async_handshake(
      ssl::stream_base::client,
      beast::bind_front_handler(
        &one_shot::on_handshake,
        shared_from_this(),
        std::move( fWrite ),
        std::move( fDone )
      )
    );
  }
}

void one_shot::on_handshake( fWriteRequest_t&& fWrite, fDone_t&& fDone, beast::error_code ec ) {

  if ( ec ) {
    fail( ec, "os.on_handshake" );
    fDone( false, ec.value(), "os.on_handshake" );
  }
  else {

    //BOOST_LOG_TRIVIAL(info) << "os.ssl_handshake";

    // Set a timeout on the operation
    beast::get_lowest_layer( m_stream ).expires_after( std::chrono::seconds( 15 ) );

    assert( fWrite );
    fWrite( std::move( fDone ) );
  }
}

void one_shot::write_empty( pRequestEmptyBody_t pRequest, fDone_t&& fDone ) {

  //BOOST_LOG_TRIVIAL(info) << "os.write_empty";

  // Send the HTTP request to the remote host
  http::async_write(
    m_stream, *pRequest,
    beast::bind_front_handler(
      &one_shot::on_write_empty,
      shared_from_this(),
      std::move( pRequest ),
      std::move( fDone )
    )
  );
}

void one_shot::write_body( pRequestStringBody_t pRequest, fDone_t&& fDone ) {

  //BOOST_LOG_TRIVIAL(info) << "os.write_body";

  // Send the HTTP request to the remote host
  http::async_write(
    m_stream, *pRequest,
    beast::bind_front_handler(
      &one_shot::on_write_body,
      shared_from_this(),
      std::move( pRequest ),
      std::move( fDone )
    )
  );
}

void one_shot::on_write_empty(
  pRequestEmptyBody_t pRequest, // keeps reference until the async has been completed
  fDone_t&& fDone,
  beast::error_code ec,
  std::size_t bytes_transferred
) {
  on_write( std::move( fDone), ec, bytes_transferred );
}

void one_shot::on_write_body(
  pRequestStringBody_t pRequest,  // keeps reference until the async has been completed
  fDone_t&& fDone,
  beast::error_code ec,
  std::size_t bytes_transferred
) {
  on_write( std::move( fDone), ec, bytes_transferred );
}

void one_shot::on_write(
  fDone_t&& fDone,
  beast::error_code ec,
  std::size_t bytes_transferred
) {
  boost::ignore_unused(bytes_transferred);

  if ( ec ) {
    fail( ec, "os.on_write" );
    fDone( false, ec.value(), "os.on_write" );
  }
  else {

    //BOOST_LOG_TRIVIAL(info) << "os.on_write";

    pDataIn_t pDataIn = std::make_shared<DataIn>();

    // Receive the HTTP response
    http::async_read(
      m_stream, pDataIn->m_buffer, pDataIn->m_parser,
      beast::bind_front_handler(
        &one_shot::on_read,
        shared_from_this(),
        std::move( pDataIn ),
        std::move( fDone )
      )
    );
  }
}

/*
   If more parser work required;

   initial inspiration:
   https://stackoverflow.com/questions/50348516/boost-beast-message-with-body-limit

   docs
   https://www.boost.org/doc/libs/1_79_0/libs/beast/doc/html/beast/using_http/parser_stream_operations.html

   https://www.vitaltrades.com/2018/12/28/reading-an-http-stream-using-c-boost-beast/ refers to the following:
   https://github.com/AndrewAMD/blog/tree/master/oanda_demo_stream

  handle chunks directly if things go horibbly wrong
   https://www.boost.org/doc/libs/1_79_0/libs/beast/doc/html/beast/ref/boost__beast__http__basic_parser.html
   https://www.boost.org/doc/libs/1_79_0/libs/beast/doc/html/beast/ref/boost__beast__http__async_read_some.html

*/

void one_shot::on_read( pDataIn_t pDataIn, fDone_t&& fDone, beast::error_code ec, std::size_t bytes_transferred ) {

  boost::ignore_unused( bytes_transferred );

  if ( ec ) {
    switch (ec.value() ) {
      case 1: // closed due to timeout, handled in regular call
        break;
      default:
        fail( ec, "os.on_read" );
        break;
    }
    fDone( false, ec.value(), "os.on_read" );
  }
  else {

    DataIn& data( *pDataIn );

    //BOOST_LOG_TRIVIAL(info) << "os.on_read";
    //BOOST_LOG_TRIVIAL(info) << "get():" << m_parser.get();
    //BOOST_LOG_TRIVIAL(info) << "body():" << m_parser.get().body();
    const auto& body = pDataIn->m_parser.get().body();

    fDone( true, ec.value(), body );
    // Set a timeout on the operation

    beast::get_lowest_layer( m_stream ).expires_after( std::chrono::seconds( 15 ) );

    // Gracefully close the stream - can the stream be re-used?
    m_stream.async_shutdown(
      beast::bind_front_handler(
        &one_shot::on_shutdown,
        shared_from_this()
      )
    );
  }
}

void one_shot::on_shutdown( beast::error_code ec ) {
  //BOOST_LOG_TRIVIAL(info) << "os.on_shutdown";
  if ( ec == asio::error::eof ) {
      // Rationale:
      // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
      ec = {};
  }
  if ( ec ) {
    //return fail( ec, "os.shutdown" );
  }

  // arrival here indicates the connection has closed gracefully
}

} // namespace session
} // namespace bot
} // namespace telegram
} // namespace ou
