
#pragma once

#include <ou/anomalon/task_group.hpp>

namespace net = boost::asio;

using executor_type = net::strand<net::io_context::executor_type>;

net::awaitable<void, executor_type>
handle_signals( task_group& task_group );