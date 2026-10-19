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
 * File:    handle_methods.cpp
 * Author:  raymond@burkholder.net
 * Project: anomalon
 * Created: July 22, 2026 18:36
 */

#include <string>

#include <boost/log/trivial.hpp>

#include "handle_methods.hpp"

namespace {
  static const std::string c_sVersion( "anomalon/1.0");

  template<typename Response>
  void response_common( Response& response ) {
    response.set( http::field::server, c_sVersion );
    response.set( "x-clacks-overhead", "GNU Terry Pratchett" );
  }

}

void response_websocket( boost::beast::websocket::response_type& response ) {
  response_common( response );
}

void response_bad_request( response_t& response, const boost::beast::string_view why ) {
  response_common( response );
  response.set( http::field::content_type, "text/plain");
  // need to filter properly for XSS issues, maybe perform escaping
  //response.body() = std::string( why );
  response.body() = "bad request\n";
}

void response_not_found( response_t& response, const boost::beast::string_view target ) {
  response_common( response );
  response.set( http::field::content_type, "text/plain");
  // need to filter properly for XSS issues, maybe perform escaping
  //response.body() = "The resource '" + std::string( target ) + "' was not found.\n";
  response.body() = "The resource was not found\n";
}

void response_server_error( response_t& response, const boost::beast::string_view what ) {
  response_common( response );
  response.set( http::field::content_type, "text/plain");
  // need to filter properly for XSS issues, maybe perform escaping
  //response.body() = "An error occurred: '" + std::string( what ) + "'\n";
  response.body() = "An error occurred\n";
}

void resource_robots_txt( response_t& response ) {
  static const std::string content( "User-agent: *\nAllow: /\n" );
  response_common( response );
  //response.set( http::field::content_type, "text/html");
  response.body() = content;
  response.content_length( content.size() );
}

void method_head( http::response<http::empty_body>& response ) {
  response_common( response );
}

void method_get( http::response<http::file_body>& response ) {
  response_common( response );
}

void method_post( response_t& response ) {
  response_common( response );
  response.set( http::field::content_type, "text/plain");
  response.body() = "your post was accepted\n";
}
