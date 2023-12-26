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
 * File:    config.hpp
 * Author:  raymond@burkholder.net
 * Project: Repertory/MQTT
 * Created: December 24, 2023 17:56:30
 */

#pragma once

#include <string>

namespace ou {
namespace mqtt {

struct Config {
  std::string sId;
  std::string sHost; // address or name (port 1833)
  std::string sUserName;
  std::string sPassword;
  std::string sTopic;
};

} // namespace mqtt
} // namespace ou
