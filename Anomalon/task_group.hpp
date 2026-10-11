/** A thread-safe task group that tracks child tasks, allows emitting
    cancellation signals to them, and waiting for their completion.
*/

#pragma once

#include <list>

#include <boost/asio.hpp>

namespace net       = boost::asio;

class task_group {

    std::mutex mtx_;
    net::steady_timer cv_;
    std::list<net::cancellation_signal> css_;

public:

    task_group(net::any_io_executor exec)
        : cv_{ std::move(exec), net::steady_timer::time_point::max() }
    {
    }

    task_group(task_group const&) = delete;
    task_group(task_group&&)      = delete;

    /** Adds a cancellation slot and a wrapper object that will remove the child
        task from the list when it completes.

        @param completion_token The completion token that will be adapted.

        @par Thread Safety
        @e Distinct @e objects: Safe.@n
        @e Shared @e objects: Safe.
    */
    template<typename CompletionToken>
    auto
    adapt(CompletionToken&& completion_token)
    {
        auto lg = std::lock_guard{ mtx_ };
        auto cs = css_.emplace(css_.end());

        class remover
        {
            task_group* tg_;
            decltype(css_)::iterator cs_;

        public:
            remover(
                task_group* tg,
                decltype(css_)::iterator cs)
                : tg_{ tg }
                , cs_{ cs }
            {
            }

            remover(remover&& other) noexcept
                : tg_{ std::exchange(other.tg_, nullptr) }
                , cs_{ other.cs_ }
            {
            }

            ~remover()
            {
                if(tg_)
                {
                    auto lg = std::lock_guard{ tg_->mtx_ };
                    if(tg_->css_.erase(cs_) == tg_->css_.end())
                        tg_->cv_.cancel();
                }
            }
        };

        return net::bind_cancellation_slot(
            cs->slot(),
            net::consign(
                std::forward<CompletionToken>(completion_token),
                remover{ this, cs }));
    }

    /** Emits the signal to all child tasks and invokes the slot's
        handler, if any.

        @param type The completion type that will be emitted to child tasks.

        @par Thread Safety
        @e Distinct @e objects: Safe.@n
        @e Shared @e objects: Safe.
    */
    void
    emit(net::cancellation_type type)
    {
        auto lg = std::lock_guard{ mtx_ };
        for(auto& cs : css_)
            cs.emit(type);
    }

    /** Starts an asynchronous wait on the task_group.

        The completion handler will be called when:

        @li All the child tasks completed.
        @li The operation was cancelled.

        @param completion_token The completion token that will be used to
        produce a completion handler. The function signature of the completion
        handler must be:
        @code
        void handler(
            boost::system::error_code const& error  // result of operation
        );
        @endcode

        @par Thread Safety
        @e Distinct @e objects: Safe.@n
        @e Shared @e objects: Safe.
    */
    template<
        typename CompletionToken =
            net::default_completion_token_t<net::any_io_executor>>
    auto
    async_wait(
        CompletionToken&& completion_token =
            net::default_completion_token_t<net::any_io_executor>{})
    {
        return net::
            async_compose<CompletionToken, void(boost::system::error_code)>(
                [this, scheduled = false](
                    auto&& self, boost::system::error_code ec = {}) mutable
                {
                    if(!scheduled)
                        self.reset_cancellation_state(
                            net::enable_total_cancellation());

                    if(!self.cancelled() && ec == net::error::operation_aborted)
                        ec = {};

                    {
                        auto lg = std::lock_guard{ mtx_ };

                        if(!css_.empty() && !ec)
                        {
                            scheduled = true;
                            return cv_.async_wait(std::move(self));
                        }
                    }

                    if(!std::exchange(scheduled, true))
                        return net::post(net::append(std::move(self), ec));

                    self.complete(ec);
                },
                completion_token,
                cv_);
    }
};
