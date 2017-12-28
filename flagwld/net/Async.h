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


#ifndef FLAGWLD_NET_ASYNC_H
#define FLAGWLD_NET_ASYNC_H

#include <ev.h>

#include <flagwld/net/Callbacks.h>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace flagwld
{
namespace net
{
class EventLoop;
typedef boost::shared_ptr<EventLoop> EventLoopPtr;

class Async : boost::noncopyable
{
 public:
  Async(const EventLoopPtr& loop);
  Async(const EventLoopPtr& loop, const AsynCallback& cb);
  ~Async();

 void setAsynCallback(const AsynCallback& cb)
  { callback_ = cb; }
#ifdef __GXX_EXPERIMENTAL_CXX0X__
  void setAsynCallback(AsynCallback&& cb)
  { callback_ = std::move(cb); }
#endif

  void start();
  void wakeup();
  void stop();

  void tie(const boost::shared_ptr<void>& obj);

 private:
  inline static void async_cb (EV_P_ ev_async* handle, int revents)
  {
    assert (revents & EV_ASYNC);
    Async* async_ptr = static_cast<Async*>(handle->data);
    assert( async_ptr );
    async_ptr->handleEvent();
  }
  void startInLoop();
  void stopInLoop();
  void handleEvent();

  bool running_;
  EventLoopPtr loop_;
  AsynCallback callback_;
  bool tied_;
  boost::weak_ptr<void> tie_;
  struct ev_async async_watcher_;
};
}
}
#endif  // FLAGWLD_NET_ASYNC_H
