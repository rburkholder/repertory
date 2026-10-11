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
 * File:    handle_methods.hpp
 * Author:  raymond@burkholder.net
 * Project: boost.web
 * Created: July 22, 2026 18:36
 */

#pragma once

#include <functional>

#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>

namespace http = boost::beast::http;

using response_t = http::response<http::string_body>;

using fResponse_t = std::function<void( response_t& )>;
using fResponseSv_t = std::function<void( response_t&, const boost::beast::string_view )>;

void resource_robots_txt( response_t& );

void response_websocket( boost::beast::websocket::response_type& );

void response_bad_request(  response_t&, const boost::beast::string_view why );
void response_not_found(    response_t&, const boost::beast::string_view target );
void response_server_error( response_t&, const boost::beast::string_view what );

void method_head( http::response<http::empty_body>& );
void method_get(  http::response<http::file_body>& );
void method_post( http::response<http::string_body>& );

void lua_method_head( std::string&, http::response<http::string_body>& );
void lua_method_get(  std::string&, http::response<http::string_body>& );
void lua_method_post( std::string&, http::response<http::string_body>& );
