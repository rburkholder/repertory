#include <boost/log/trivial.hpp>

#include "handle_signals.hpp"

net::awaitable<void, executor_type>
handle_signals( task_group& task_group ) {

  auto executor   = co_await net::this_coro::executor;
  auto signal_set = net::signal_set{ executor, SIGINT, SIGTERM };

  auto signal = co_await signal_set.async_wait();

  switch ( signal ) {
    case SIGINT:
      {
        BOOST_LOG_TRIVIAL(info) << "Gracefully cancelling child tasks...\n";
        task_group.emit( net::cancellation_type::total );

        // Wait a limited time for child tasks to gracefully cancell
        auto [ec] = co_await task_group.async_wait(
          net::as_tuple(net::cancel_after(std::chrono::seconds{ 10 })));

        if(ec == net::error::operation_aborted) { // Timeout occurred
          BOOST_LOG_TRIVIAL(info) << "Sending a terminal cancellation signal...\n";
          task_group.emit( net::cancellation_type::terminal );
          co_await task_group.async_wait();
        }

        BOOST_LOG_TRIVIAL(info) << "Child tasks completed.\n";
      }
      break;
    default: // SIGTERM
      {
        net::query(
          executor.get_inner_executor(),
          net::execution::context).stop();
      }
      break;
  }
}

