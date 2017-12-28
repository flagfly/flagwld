/*
   Copyright (C) Zhang GuoQi <guoqi.zhang@gmail.com>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   see https://github.com/chenshuo/muduo/
   see http://software.schmorp.de/pkg/libev.html

   libev was written and designed by Marc Lehmann and Emanuele Giaquinta.
   muduo was written by chenshuo.
*/


#include <flagwld/net/EventLoop.h>

#include <flagwld/base/Logging.h>
#include <flagwld/base/Mutex.h>

#include <boost/bind.hpp>

#include <unistd.h>

using namespace flagwld;
using namespace flagwld::net;

namespace
{
  __thread EventLoop* t_loopInThisThread = 0;
}

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
  return t_loopInThisThread;
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
EventLoop::EventLoop()
  : looping_(false),
    quit_(true),
    threadId_(CurrentThread::tid()),
    loop_( (::getpid() == threadId_)?ev_default_loop(0):ev_loop_new(0))
{
  LOG_DEBUG << "EventLoop created " << this << " in thread " << threadId_;

  if (t_loopInThisThread)
  {
    LOG_FATAL << "Another EventLoop " << t_loopInThisThread
              << " exists in this thread " << threadId_;
  }
  else
  {
    t_loopInThisThread = this;
  }
  async_watcher_.data = this;
  ev_async_init(&async_watcher_, async_cb);
}
#pragma GCC diagnostic error "-Wold-style-cast"

EventLoop::~EventLoop()
{
  LOG_DEBUG << "EventLoop " << this << " of thread " << threadId_
            << " destructs in thread " << CurrentThread::tid();

  assert( quit_ );

  ev_loop_destroy(loop_);
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
void EventLoop::loop()
{
  assert(!looping_);
  assertInLoopThread();
  looping_ = true;
  quit_ = false;
  LOG_TRACE << "EventLoop " << this << " start looping";

  ev_async_start(loop_, &async_watcher_);

  while (!quit_)
  {
    ev_run(loop_, 0);
  }

  ev_async_stop(loop_, &async_watcher_);

  LOG_TRACE << "EventLoop " << this << " stop looping";
  looping_ = false;
}

void EventLoop::quit()
{
  quit_ = true;
  ev_async_send(loop_, &async_watcher_);
}
#pragma GCC diagnostic error "-Wold-style-cast"

void EventLoop::runInLoop(const std::vector<Functor> &cbs)
{
  if (isInLoopThread())
  {
    for (size_t i = 0; i < cbs.size(); ++i)
    { 
      cbs[i]();
    }
  }
  else
  {
    queueInLoop(cbs);
  }
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
void EventLoop::queueInLoop(const std::vector<Functor> &cbs)
{
  {
    MutexLockGuard lock(mutex_);
    for (size_t i = 0; i < cbs.size(); ++i)
    {
      pendingFunctors_.push_back( cbs[i] );
    }
  }
  ev_async_send(loop_, &async_watcher_);
}
#pragma GCC diagnostic error "-Wold-style-cast"

void EventLoop::runInLoop(const Functor& cb)
{ 
  if (isInLoopThread())
  { 
    cb();
  }
  else
  { 
    queueInLoop(cb);
  }
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
void EventLoop::queueInLoop(const Functor& cb)
{ 
  { 
    MutexLockGuard lock(mutex_);
    pendingFunctors_.push_back(cb);
  }
   
  ev_async_send(loop_, &async_watcher_);
}
#pragma GCC diagnostic error "-Wold-style-cast"

#ifdef __GXX_EXPERIMENTAL_CXX0X__
void EventLoop::runInLoop(Functor&& cb)
{
  if (isInLoopThread())
  {
    cb();
  }
  else
  {
    queueInLoop(std::move(cb));
  }
}

void EventLoop::runInLoop(std::vector<Functor>&& cbs)
{
  if (isInLoopThread())
  {
    for (size_t i = 0; i < cbs.size(); ++i)
    {
      cbs[i]();
    }
  }
  else
  {
    queueInLoop(std::move(cbs));
  }
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
void EventLoop::queueInLoop(Functor&& cb)
{
  {
    MutexLockGuard lock(mutex_);
    pendingFunctors_.push_back(std::move(cb));
  }

  ev_async_send(loop_, &async_watcher_);
}
#pragma GCC diagnostic error "-Wold-style-cast"

#pragma GCC diagnostic ignored "-Wold-style-cast"
void EventLoop::queueInLoop(std::vector<Functor>&& cbs)
{
  {
    MutexLockGuard lock(mutex_);
    for (size_t i = 0; i < cbs.size(); ++i)
    {
      pendingFunctors_.push_back( std::move(cbs[i]) );
    }
  }
  ev_async_send(loop_, &async_watcher_);
}
#pragma GCC diagnostic error "-Wold-style-cast"
#endif

size_t EventLoop::queueSize() const
{
  MutexLockGuard lock(mutex_);
  return pendingFunctors_.size();
}

void EventLoop::abortNotInLoopThread()
{
  LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
            << " was created in threadId_ = " << threadId_
            << ", current thread id = " <<  CurrentThread::tid();
}

void EventLoop::doPendingFunctors()
{
  LOG_TRACE << "EventLoop " << this << " Enter in";

  std::vector<Functor> functors;
  {
  MutexLockGuard lock(mutex_);
  functors.swap(pendingFunctors_);
  }

  for (size_t i = 0; i < functors.size(); ++i)
  {
    functors[i]();
  }

  if(quit_)
  {
    ev_break(loop_, EVBREAK_ALL);
    LOG_TRACE << "EventLoop " << this << " stopping looping";
  }
}
