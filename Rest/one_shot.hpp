/************************************************************************
 * Copyright(c) 2023, One Unified. All rights reserved.                 *
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
 * File:    one_shot.hpp
 * Project: Repertory/Telegram
 * Author:  raymond@burkholder.net
 * Created: February 17, 2023 10:02:25
 */

#pragma once

#include "handler.hpp"

namespace ou {
namespace rest {

// https://www.boost.org/doc/libs/1_79_0/libs/beast/example/http/client/async-ssl/http_client_async_ssl.cpp
class one_shot : ou::rest::handler {
public:

  explicit one_shot(
    asio::any_io_executor,
    ssl::context&
  );
  virtual ~one_shot();

  void run(
    const std::string& sHost
  , const std::string& sPort
  , const std::string& sTarget
  , int version
  );

  void get(
    const std::string& sHost
  , const std::string& sPort
  , const std::string& sTarget
  , fDone_t&&
  );

  void get(
    const std::string& sHost
  , const std::string& sPort
  , const std::string& sTarget
  , const std::string& sBody
  , fDone_t&&
  );

  void post(
    const std::string& sHost
  , const std::string& sPort
  , const std::string& sTarget
  , const std::string& sBody
  , fDone_t&&
  );

  void patch(
    const std::string& sHost
  , const std::string& sPort
  , const std::string& sTarget
  , const std::string& sBody
  , fDone_t&&
  );

  void delete_(
    const std::string& sHost
  , const std::string& sPort
  , const std::string& sTarget
  , fDone_t&&
  );

private:
};

} // namespace rest
} // namespace ou
