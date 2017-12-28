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


#ifndef FLAGWLD_NET_EVENTLOOP_H
#define FLAGWLD_NET_EVENTLOOP_H

#include <flagwld/base/Mutex.h>
#include <flagwld/base/CurrentThread.h>
#include <flagwld/base/Timestamp.h>
#include <flagwld/net/Callbacks.h>

#include <ev.h>

#include <boost/any.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

#include <vector>

namespace flagwld
{
namespace net
{

class EventLoop : boost::noncopyable
{
 public:
  typedef boost::function<void()> Functor;

  EventLoop();
  ~EventLoop();

  void loop();
  void quit();

  void runInLoop(const Functor& cb);
  void runInLoop(const std::vector<Functor>& cbs);

  void queueInLoop(const Functor& cb);
  void queueInLoop(const std::vector<Functor>& cbs);

#ifdef __GXX_EXPERIMENTAL_CXX0X__
  void runInLoop(Functor&& cb);
  void runInLoop(std::vector<Functor>&& cbs);
  void queueInLoop(Functor&& cb);
  void queueInLoop(std::vector<Functor>&& cbs);
#endif

  size_t queueSize() const;

  void assertInLoopThread()
  {
    if (!isInLoopThread())
    {
      abortNotInLoopThread();
    }
  }

  bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

  void setContext(const boost::any& context)
  { context_ = context; }

  const boost::any& getContext() const
  { return context_; }

  boost::any* getMutableContext()
  { return &context_; }

  static EventLoop* getEventLoopOfCurrentThread();

 private:
  friend class Signal;
  friend class Async;
  friend class Idle;
  friend class Timer;
  friend class Periodic;
  friend class Channel;
  friend class Child;
  inline static void async_cb (EV_P_ ev_async* handle, int status)
  {
    EventLoop* loop_ptr = static_cast<EventLoop*>(handle->data);
    assert( loop_ptr );
    loop_ptr->doPendingFunctors();
  }
  void doPendingFunctors();
  void abortNotInLoopThread();

  bool looping_;
  bool quit_;
  pid_t threadId_;
   
  struct ev_loop* loop_;

  struct ev_async async_watcher_;

  boost::any context_;

  mutable MutexLock mutex_;
  std::vector<Functor> pendingFunctors_; // @GuardedBy mutex_
};

typedef boost::shared_ptr<EventLoop> EventLoopPtr;
}
}
#endif  // FLAGWLD_NET_EVENTLOOP_H
