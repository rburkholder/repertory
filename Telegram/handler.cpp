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
          &handler::on_connect,
          shared_from_this(),
          std::move( fWrite ),
          std::move( fDone )
        )
      );
  }
}

void handler::on_connect(
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
        &handler::on_handshake,
        shared_from_this(),
        std::move( fWrite ),
        std::move( fDone )
      )
    );
  }
}

void handler::on_handshake( fWriteRequest_t&& fWrite, fDone_t&& fDone, beast::error_code ec ) {

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

void handler::write_empty( pRequestEmptyBody_t pRequest, fDone_t&& fDone ) {

  //BOOST_LOG_TRIVIAL(info) << "os.write_empty";

  // Send the HTTP request to the remote host
  http::async_write(
    m_stream, *pRequest,
    beast::bind_front_handler(
      &handler::on_write_empty,
      shared_from_this(),
      std::move( pRequest ),
      std::move( fDone )
    )
  );
}

void handler::write_body( pRequestStringBody_t pRequest, fDone_t&& fDone ) {

  //BOOST_LOG_TRIVIAL(info) << "os.write_body";

  // Send the HTTP request to the remote host
  http::async_write(
    m_stream, *pRequest,
    beast::bind_front_handler(
      &handler::on_write_body,
      shared_from_this(),
      std::move( pRequest ),
      std::move( fDone )
    )
  );
}

void handler::on_write_empty(
  pRequestEmptyBody_t pRequest, // keeps reference until the async has been completed
  fDone_t&& fDone,
  beast::error_code ec,
  std::size_t bytes_transferred
) {
  on_write( std::move( fDone), ec, bytes_transferred );
}

void handler::on_write_body(
  pRequestStringBody_t pRequest,  // keeps reference until the async has been completed
  fDone_t&& fDone,
  beast::error_code ec,
  std::size_t bytes_transferred
) {
  on_write( std::move( fDone), ec, bytes_transferred );
}

void handler::on_write(
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
        &handler::on_read,
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

void handler::on_read( pDataIn_t pDataIn, fDone_t&& fDone, beast::error_code ec, std::size_t bytes_transferred ) {

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
