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
 * File:    handler.hpp
 * Project: Repertory/Telegram
 * Author:  raymond@burkholder.net
 * Created: September 19, 2026 23:39:57
 */

#pragma once

#include <memory>
#include <string>
#include <variant>

#include <boost/beast/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

namespace asio  = boost::asio;      // from <boost/asio.hpp>
namespace ssl   = asio::ssl;        // from <boost/asio/ssl.hpp>

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http  = beast::http;      // from <boost/beast/http.hpp>

using tcp = boost::asio::ip::tcp;   // from <boost/asio/ip/tcp.hpp>

namespace ou {
namespace rest {

class handler
: public std::enable_shared_from_this<handler>
{
public:

  using fDone_t = std::function<void(bool,int,const std::string&)>; // false, not ok; true, fine

  handler(
    asio::any_io_executor,
    ssl::context&
  );
  virtual ~handler();

protected:

  tcp::resolver m_resolver;
  beast::ssl_stream<beast::tcp_stream> m_stream;

  using pRequestEmptyBody_t = std::shared_ptr<http::request<http::empty_body> >; // unique_ptr doesn't work in the bind
  using pRequestStringBody_t = std::shared_ptr<http::request<http::string_body> >; // unique_ptr doesn't work in the bind

  using pRequestBody_t = std::variant<pRequestEmptyBody_t, pRequestStringBody_t>;
  pRequestBody_t m_pRequestBody;

  void write_empty();
  void write_body();

  void on_resolve( beast::error_code, tcp::resolver::results_type );

  fDone_t m_fDone;

private:

  void on_connect( beast::error_code, tcp::resolver::results_type::endpoint_type );
  void on_handshake( beast::error_code );

  void on_write( beast::error_code, std::size_t bytes_transferred );

  struct DataIn {
    beast::flat_buffer m_buffer; // (Must persist between reads)
    http::response_parser<http::string_body> m_parser;
    DataIn() {
     // Allow for an unlimited body size
      m_parser.body_limit( ( std::numeric_limits<std::uint64_t>::max )() );
    }
    ~DataIn() {
      m_buffer.clear();
    }
  };
  DataIn m_DataIn;

  void on_read( beast::error_code, std::size_t bytes_transferred );

  void on_shutdown( beast::error_code ec );

};

} // namespace rest
} // namespace ou
