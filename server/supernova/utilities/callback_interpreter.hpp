//  templated callback system
//  Copyright (C) 2008, 2009 Tim Blechmann
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; see the file COPYING.  If not, write to
//  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
//  Boston, MA 02111-1307, USA.

#ifndef UTILITIES_CALLBACK_INTERPRETER_HPP
#define UTILITIES_CALLBACK_INTERPRETER_HPP

#include "branch_hints.hpp"
#include "callback_system.hpp"

#include "nova-tt/semaphore.hpp"
#include "nova-tt/thread_priority.hpp"

#include <boost/atomic.hpp>
#include <boost/bind.hpp>
#include <boost/checked_delete.hpp>
#include <boost/ref.hpp>
#include <boost/thread.hpp>


namespace nova
{

namespace detail
{
struct nop_functor
{
    void operator()() const {}
};
} /* namespace detail */



template <class callback_type,
          class callback_deleter = boost::checked_deleter<callback_type> >
class callback_interpreter:
    callback_system<callback_type>
{
public:
    callback_interpreter(void):
        sem(0), running(false)
    {}

    void add_callback(callback_type * cb)
    {
        callback_system<callback_type>::add_callback(cb);
        sem.post();
    }

    void run(void)
    {
        running.store(true, boost::memory_order_relaxed);
        perform();
    }

    void terminate(void)
    {
        running.store(false, boost::memory_order_relaxed);
        sem.post();
    }

protected:
    void run(semaphore & sync_sem)
    {
        running.store(true, boost::memory_order_relaxed);
        sync_sem.post();
        perform();
    }

private:
    void perform(void)
    {
        do
        {
            sem.wait();
            callback_system<callback_type>::run_callbacks();
        }
        while(likely(running.load(boost::memory_order_relaxed)));
    }

protected:
    semaphore sem;
    boost::atomic<bool> running;
};


template <class callback_type,
          class init_functor = detail::nop_functor,
          class callback_deleter = boost::checked_deleter<callback_type> >
class threaded_callback_interpreter:
    public callback_interpreter<callback_type, callback_deleter>,
    init_functor
{
    typedef callback_interpreter<callback_type, callback_deleter> super;

    boost::thread thread;

public:
    threaded_callback_interpreter(void)
    {}

    ~threaded_callback_interpreter(void)
    {
        if (super::running.load())
            join_thread();
    }

    void start_thread(void)
    {
        semaphore sync_sem;
        semaphore_sync sync(sync_sem);
        boost::thread thr(boost::bind(&threaded_callback_interpreter::run_thread, this, boost::ref(sync_sem)));
        thread = thr.move();
    }

    void run_thread(semaphore & sync_sem)
    {
        init_functor::operator()();
        super::run(sync_sem);
    }

    void join_thread(void)
    {
        super::terminate();
        thread.join();
    }
};

template <class callback_type,
          class init_functor = detail::nop_functor,
          class callback_deleter = boost::checked_deleter<callback_type> >
class callback_interpreter_threadpool:
    public callback_interpreter<callback_type, callback_deleter>,
    init_functor
{
    typedef callback_interpreter<callback_type, callback_deleter> super;

public:
    callback_interpreter_threadpool(uint16_t worker_thread_count, bool rt, uint16_t priority):
        worker_thread_count_(worker_thread_count), priority(priority), rt(rt)
    {
        semaphore sync_sem;
        for (uint16_t i = 0; i != worker_thread_count; ++i)
            threads.create_thread(boost::bind(&callback_interpreter_threadpool::run, this, boost::ref(sync_sem)));

        for (uint16_t i = 0; i != worker_thread_count; ++i)
            sync_sem.wait();
    }

    ~callback_interpreter_threadpool(void)
    {
        if (super::running.load())
            join_threads();
    }

    void join_threads(void)
    {
        super::running.store(false);

        for (uint16_t i = 0; i != worker_thread_count_; ++i)
            super::sem.post();
        threads.join_all();
    }

private:
    void run(semaphore & sync_sem)
    {
#ifdef NOVA_TT_PRIORITY_RT
        if (rt)
            thread_set_priority_rt(priority);
        else
#endif
            thread_set_priority(priority);

        init_functor::operator()();
        super::run(sync_sem);
    }

    boost::thread_group threads;
    uint16_t worker_thread_count_;
    uint16_t priority;
    bool rt;
};


} /* namespace nova */

#endif /* UTILITIES_CALLBACK_INTERPRETER_HPP */
