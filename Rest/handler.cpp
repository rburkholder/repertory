/************************************************************************
 * Copyright(c) 2026, One Unified. All rights reserved.                 *
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
 * File:    handler.cpp
 * Project: Repertory/Telegram
 * Author:  raymond@burkholder.net
 * Created: September 19, 2026 23:39:57
 */

#include <boost/log/trivial.hpp>

#include "handler.hpp"

namespace ou {
namespace rest {

// Report a failure
void fail( beast::error_code ec, char const* what ) {
  BOOST_LOG_TRIVIAL(error) << what << ": (" << ec.value() << ") " << ec.message();
}

handler::handler(
  asio::any_io_executor ex,
  ssl::context& ssl_ctx
)
: m_resolver( ex )
, m_stream( ex, ssl_ctx )
{
}

handler::~handler() {
  //m_stream.shutdown();  // doesn't like this
}

void handler::on_resolve(
  beast::error_code ec,
  tcp::resolver::results_type results
) {
  if ( ec ) {
    fail( ec, "os.on_resolve");
    m_fDone( false, ec.value(), "os.on_resolve" );
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
          &handler::on_connect,
          shared_from_this()
        )
      );
  }
}

void handler::on_connect(
  beast::error_code ec, tcp::resolver::results_type::endpoint_type et
) {
  if ( ec ) {
    fail( ec, "os.on_connect" );
    m_fDone( false, ec.value(), "os.on_connect" );
  }
  else {

    //BOOST_LOG_TRIVIAL(info) << "os.on_connect";

    // Perform the SSL handshake
    m_stream.async_handshake(
      ssl::stream_base::client,
      beast::bind_front_handler(
        &handler::on_handshake,
        shared_from_this()
      )
    );
  }
}

void handler::on_handshake( beast::error_code ec ) {

  if ( ec ) {
    fail( ec, "os.on_handshake" );
    m_fDone( false, ec.value(), "os.on_handshake" );
  }
  else {

    //BOOST_LOG_TRIVIAL(info) << "os.ssl_handshake";

    // Set a timeout on the operation
    beast::get_lowest_layer( m_stream ).expires_after( std::chrono::seconds( 15 ) );

    switch ( m_pRequestBody.index() ) {
      case 0:
        write_empty();
        break;
      case 1:
        write_body();
        break;
      default:
        break;
    }
  }
}

void handler::write_empty() {

  //BOOST_LOG_TRIVIAL(info) << "os.write_empty";

  pRequestEmptyBody_t& pRequest = std::get<pRequestEmptyBody_t>( m_pRequestBody );

  // Send the HTTP request to the remote host
  http::async_write(
    m_stream, *pRequest,
    beast::bind_front_handler(
      &handler::on_write,
      shared_from_this()
    )
  );
}

void handler::write_body() {

  //BOOST_LOG_TRIVIAL(info) << "os.write_body";

  pRequestStringBody_t& pRequest = std::get<pRequestStringBody_t>( m_pRequestBody );

  // Send the HTTP request to the remote host
  http::async_write(
    m_stream, *pRequest,
    beast::bind_front_handler(
      &handler::on_write,
      shared_from_this()
    )
  );
}

void handler::on_write(
  beast::error_code ec,
  std::size_t bytes_transferred
) {
  boost::ignore_unused(bytes_transferred);

  if ( ec ) {
    fail( ec, "os.on_write" );
    m_fDone( false, ec.value(), "os.on_write" );
  }
  else {

    //BOOST_LOG_TRIVIAL(info) << "os.on_write";

    // Receive the HTTP response
    http::async_read(
      m_stream, m_DataIn.m_buffer, m_DataIn.m_parser,
      beast::bind_front_handler(
        &handler::on_read,
        shared_from_this()
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

void handler::on_read( beast::error_code ec, std::size_t bytes_transferred ) {

  boost::ignore_unused( bytes_transferred );

  if ( ec ) {
    switch (ec.value() ) {
      case 1: // closed due to timeout, handled in regular call
        break;
      default:
        fail( ec, "os.on_read" );
        break;
    }
    m_fDone( false, ec.value(), "os.on_read" );
  }
  else {

    //BOOST_LOG_TRIVIAL(info) << "os.on_read";
    //BOOST_LOG_TRIVIAL(info) << "get():" << m_parser.get();
    //BOOST_LOG_TRIVIAL(info) << "body():" << m_parser.get().body();
    const auto& body = m_DataIn.m_parser.get().body();

    m_fDone( true, ec.value(), body );
    // Set a timeout on the operation

    beast::get_lowest_layer( m_stream ).expires_after( std::chrono::seconds( 15 ) );

    // Gracefully close the stream - can the stream be re-used?
    m_stream.async_shutdown(
      beast::bind_front_handler(
        &handler::on_shutdown,
        shared_from_this()
      )
    );
  }
}

void handler::on_shutdown( beast::error_code ec ) {
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

} // namespace rest
} // namespace ou
