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
 * File:    handle_methods_lua.hpp
 * Author:  raymond@burkholder.net
 * Project: anomalon
 * Created: October 18, 2026 18:38:32
 */

#pragma once

#include <functional>

#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>

#include <boost/url/params_view.hpp>

namespace sol {
  class state;
}

namespace http = boost::beast::http;

using response_t = http::response<http::string_body>;

// loads custom code per thread instance
using fLoadCustomCode_t = std::function<void(sol::state&)>;
void LoadCustomCode( fLoadCustomCode_t&& );

void lua_method_head( const std::string&, const boost::urls::params_view&, http::response<http::string_body>& );
void lua_method_get(  const std::string&, const boost::urls::params_view&, http::response<http::string_body>& );
void lua_method_post( const std::string&, const boost::urls::params_view&, http::response<http::string_body>& );
