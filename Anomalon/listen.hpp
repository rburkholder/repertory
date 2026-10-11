#pragma once

#include <string>

#include <boost/asio/ssl/context.hpp>

#include <boost/beast/core.hpp>

#include "task_group.hpp"

namespace net = boost::asio;
namespace ssl = boost::asio::ssl;

using executor_type = net::strand<net::io_context::executor_type>;

net::awaitable<void, executor_type>
listen(
  task_group&,
  ssl::context&,
  net::ip::tcp::endpoint,
  const std::string& sContentDirectory
);