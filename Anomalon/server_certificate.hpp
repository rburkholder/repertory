//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_BEAST_EXAMPLE_COMMON_SERVER_CERTIFICATE_HPP
#define BOOST_BEAST_EXAMPLE_COMMON_SERVER_CERTIFICATE_HPP

#include <boost/asio/ssl/context.hpp>

/* Load a signed certificate into the ssl context, and configure
   the context for use with a server.
*/

void
load_server_certificate(
  boost::asio::ssl::context& ctx
, const std::string& sPathFullChain
, const std::string& sPathPrivKey
);

#endif
