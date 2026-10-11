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
 * File:    mime_type.hpp
 * Author:  raymond@burkholder.net
 * Project: boost.web
 * Created: July 22, 2026 18:36
 */

#pragma once

#include <boost/beast/core/string.hpp>

namespace beast = boost::beast;

class mime_type {
public:

  enum class type_t {
    html = 1, md, lua, image, xml, css, text, flash, js, json, icon, svg, csv, unknown
  };

  struct entry_t {
    type_t type;
    beast::string_view name;
    entry_t( type_t type_, beast::string_view name_ )
    : type( type_ ), name( name_ )
    {}
    entry_t(): type( type_t::unknown ), name( "application/octet-stream" ) {}
  };

  mime_type();
  ~mime_type();

  // Return a reasonable mime type based on the extension of a file.
  const entry_t lu( const beast::string_view ) const;
protected:
private:
};
