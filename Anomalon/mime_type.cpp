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
 * File:    mime_type.cpp
 * Author:  raymond@burkholder.net
 * Project: boost.web
 * Created: July 22, 2026 18:36
 */

#include <boost/parser/parser.hpp>

#include "mime_type.hpp"

// more types can be found in /etc/mime.types

namespace {
  boost::parser::symbols<mime_type::entry_t> const entries = {
    { "htm",  mime_type::entry_t( mime_type::type_t::html,  "text/html" ) },
    { "html", mime_type::entry_t( mime_type::type_t::html,  "text/html" ) },
    { "php",  mime_type::entry_t( mime_type::type_t::html,  "text/html" ) },
    { "dyn",  mime_type::entry_t( mime_type::type_t::lua,   "text/html" ) },
    { "lua",  mime_type::entry_t( mime_type::type_t::lua,   "text/html" ) },
    { "md",   mime_type::entry_t( mime_type::type_t::md,    "text/markdown" ) },
    { "css",  mime_type::entry_t( mime_type::type_t::css,   "text/css" ) },
    { "txt",  mime_type::entry_t( mime_type::type_t::text,  "text/plain" ) },
    { "text", mime_type::entry_t( mime_type::type_t::text,  "text/plain" ) },
    { "csv",  mime_type::entry_t( mime_type::type_t::csv,   "text/csv" ) },
    { "js",   mime_type::entry_t( mime_type::type_t::js,    "application/javascript" ) },
    { "json", mime_type::entry_t( mime_type::type_t::json,  "application/json" ) },
    { "xml",  mime_type::entry_t( mime_type::type_t::xml,   "application/xml" ) },
    { "swf",  mime_type::entry_t( mime_type::type_t::flash, "application/x-shockwave-flash" ) },
    { "flv",  mime_type::entry_t( mime_type::type_t::flash, "video/x-flv" ) },
    { "png",  mime_type::entry_t( mime_type::type_t::image, "image/png" ) },
    { "jpe",  mime_type::entry_t( mime_type::type_t::image, "image/jpeg" ) },
    { "jpeg", mime_type::entry_t( mime_type::type_t::image, "image/jpeg" ) },
    { "jpg",  mime_type::entry_t( mime_type::type_t::image, "image/jpeg" ) },
    { "gif",  mime_type::entry_t( mime_type::type_t::image, "image/gif" ) },
    { "bmp",  mime_type::entry_t( mime_type::type_t::image, "image/bmp" ) },
    { "ico",  mime_type::entry_t( mime_type::type_t::icon,  "image/vnd.microsoft.icon" ) },
    { "tiff", mime_type::entry_t( mime_type::type_t::image, "image/tiff" ) },
    { "tif",  mime_type::entry_t( mime_type::type_t::image, "image/tiff" ) },
    { "svg",  mime_type::entry_t( mime_type::type_t::svg,   "image/svg+xml" ) },
    { "svgz", mime_type::entry_t( mime_type::type_t::svg,   "image/svg+xml" ) }
  };
}

mime_type::mime_type() {
}

mime_type::~mime_type() {
}

const mime_type::entry_t mime_type::lu( const beast::string_view path ) const {

  auto const ext = [&path] {
    auto const pos = path.rfind( '.' );
    if( pos == beast::string_view::npos )
      return beast::string_view{};
    if ( pos == ( path.size() - 1 ) ) {
      return beast::string_view();
    }
    return path.substr( pos + 1 );
  }();

  entry_t entry;

  auto const f_suffix = [&entry]( auto& ctx ) {
    entry = _attr( ctx );
  };
  auto const parser = +entries[ f_suffix ] >> boost::parser::eoi;
  bool result = boost::parser::parse( ext, parser );
  if ( !result ) {
    return entry_t();
  }

  return entry;

}

