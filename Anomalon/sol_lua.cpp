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
 * File:    sol_lua.cpp
 * Author:  raymond@burkholder.net
 * Project: boost.web
 * Created: August 5, 2026 15:47:57
 */

#include <boost/log/trivial.hpp>

#include "sol_lua.hpp"

namespace {
  const auto& instance_identify = R"(
    local instance, name = ...
    print( 'lua session ' .. instance .. ' ' .. name )
  )";
}

std::atomic_uint64_t sol_lua_t::m_nInstanceCounter( 1 );

sol_lua_t::sol_lua_t()
: m_nInstance( m_nInstanceCounter.fetch_add( 1, std::memory_order_relaxed ) )
{
  // maybe only open libraries when required?
  m_sol.open_libraries( sol::lib::base, sol::lib::package, sol::lib::table, sol::lib::string );
  m_sol.set_function( "print", &sol_lua_t::print );
  f_instance_identify = m_sol.load( instance_identify );
  if ( f_instance_identify.valid() ) {
    f_instance_identify( m_nInstance, "begin" );
  }
}

sol_lua_t::~sol_lua_t() {
  if ( f_instance_identify.valid() ) {
    f_instance_identify( m_nInstance, "end" );
  }
}

void sol_lua_t::print( const std::string_view message ) {
  BOOST_LOG_TRIVIAL(info) << message;
}
