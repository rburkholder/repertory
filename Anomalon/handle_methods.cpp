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
 * Project: boost.web
 * Created: July 22, 2026 18:36
 */

// TODO
/*
sol::thread lua_th = sol::thread::create(master);
sol::state_view local_lua_state = lua_th.state();

sol::protected_function handler = local_lua_state["route_http"];
if (handler.valid()) {
    auto result = co_await handler(path); // Safely executed on an isolated stack
    if (result.valid()) {
        body = result.get<std::string>();
    }
}
*/

#include <chrono>
#include <string>

#include <boost/log/trivial.hpp>

#include <fmt/chrono.h>

#include "sol_lua.hpp"
#include "handle_methods.hpp"

// how-to: build folder with index.lua.  folder is the long name for seo, auto runs the index.lua file for content
//   folder name is redirect to index file?

namespace {
  //static const std::string c_sVersion( "ounl-lua/1.0");
  static const std::string c_sVersion( "anomalon/1.0");

  // each needs to be thread safe, maintain state in a different structure
  thread_local sol_lua_t sol_lua;

  template<typename Response>
  void response_common( Response& response ) {
    response.set( http::field::server, c_sVersion );
    response.set( "x-clacks-overhead", "GNU Terry Pratchett" );
  }

}

void LoadCustomCode( fLoadCustomCode_t&& f ) {
  sol_lua.LoadCustomCode( std::move( f ) );
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

// TODO: convert read in to async operation, coroutine?, async load file(s), then pass as text based scripts
//   pre-compile to shared memory for re-use by luajit across sessions
//   use file change to match against cached contents, don't pre-cache though
void lua_method_get( const std::string& path, const boost::urls::params_view& params, http::response<http::string_body>& response ) {

  response_common( response );

  sol_lua().set_function(
    "GetTimeStamp",
    []()->std::string {
      const auto now = floor<std::chrono::seconds>( std::chrono::utc_clock::now() );
      // Wed, 09 Jun 2021 10:18:14 GMT
      const auto dt1( fmt::format( "{:%a, %d %b %Y %H:%M:%S} GMT", now ) );
      const auto dt2( fmt::format( "{:%Y%m%d%H%M%S}", now ) );
      return dt1 + '-' + dt2;
    }
  );

  // Register the boost params type with sol2
  sol_lua().new_usertype<boost::urls::params_view>(
    "url_params",
    // Allow Lua to index keys directly: url_params["q"]
    sol::meta_function::index,
    []( boost::urls::params_view& p, std::string key ) -> sol::object {
      auto it = p.find( key );
      if (it != p.end() && (*it).has_value) {
        return sol::make_object(sol_lua(), (*it).value);
      }
      return sol::nil;
    }
  );

  try {

    auto result = sol_lua().safe_script_file(
      path,
      []( lua_State*, sol::protected_function_result pfr ){
        sol::error err = std::move( pfr );
        BOOST_LOG_TRIVIAL(error) << "An error (an expected one) occurred: " << err.what();
        return pfr;
      }
    );
    if ( result.valid() ) {
      sol_lua()[ "url_params" ] = params;
      std::string mime_type;
      std::string body; // is this an error, is the original still available?
      sol::tie( mime_type, body ) = sol_lua()[ "render" ]();
      response.set( http::field::content_type, mime_type );
      response.body() = body;
      response.content_length( body.length() );
      {
        // todo: re-use previous cookie if present
        const auto now( floor<std::chrono::seconds>( std::chrono::utc_clock::now() ) );
        const auto dtNow( fmt::format( "{:%Y%m%d%H%M%S}", now ) );
        const auto expiry( now + std::chrono::days( 7 ) );
        // Wed, 09 Jun 2021 10:18:14 GMT
        const auto dtExpire( fmt::format( "{:%a, %d %b %Y %H:%M:%S} GMT", expiry ) );
        // use comma to separate multiple cookies
        response.insert( http::field::set_cookie, "TS=" + dtNow + ";Path=/;Expires=" + dtExpire );
        response.insert( http::field::set_cookie, "EOC=another;Path=/;Expires=" + dtExpire );
      }
      return;
    }
    else {
      //BOOST_LOG_TRIVIAL(error) << "lua script result: false, " << result.status();
      BOOST_LOG_TRIVIAL(error) << "lua script result: false";
    }
  }
  catch ( const sol::error& e ) {
    BOOST_LOG_TRIVIAL(error) << "lua script error: " << e.what();
  }

  // default error response
  response.result( http::status::not_found );
  response.set( http::field::content_type, "text/plain");
  response.body() = "The resource '" + path + "' has error\n";
  response.prepare_payload();

}

void lua_method_head( const std::string& path, const boost::urls::params_view& params, http::response<http::string_body>& response ) {

  response_common( response );

  sol::load_result script;
  try {
    script = sol_lua().load_file( path, sol::load_mode::text );
    if ( script.valid() ) {
      response.set( http::field::content_type, "text/html" );
      response.content_length( 0 );
      return;
    }
  }
  catch ( const sol::error& e ) {
    BOOST_LOG_TRIVIAL(error) << "lua script error (1): " << e.what();
  }

  response.result( http::status::not_found );
  response.set( http::field::content_type, "text/plain");
  response.body() = "The resource '" + path + "' has error\n";
  response.prepare_payload();
}

void lua_method_post( const std::string& path, const boost::urls::params_view& params, http::response<http::string_body>& response ) {

  response_common( response );

  sol_lua().set_function(
    "Render",
    [&response]( const std::string_view type, const std::string_view src ){
      response.set( http::field::content_type, type );
      response.body() = src;
      response.content_length( src.length() );
    } );

  // todo: pull out body variables and pass to script

  try {
    auto result = sol_lua().safe_script_file(
      path,
      []( lua_State*, sol::protected_function_result pfr ){
        sol::error err = std::move( pfr );
        BOOST_LOG_TRIVIAL(error) << "An error (an expected one) occurred: " << err.what();
        return pfr;
      }
    );
    if ( result.valid() ) {
      return;
    }
    else {
      BOOST_LOG_TRIVIAL(error) << "lua script result: false";
    }
  }
  catch ( const sol::error& e ) {
    BOOST_LOG_TRIVIAL(error) << "lua script error: " << e.what();
  }

  // default error response
  response.result( http::status::not_found );
  response.set( http::field::content_type, "text/plain");
  response.body() = "The resource '" + path + "' has error\n";
  response.prepare_payload();
}
