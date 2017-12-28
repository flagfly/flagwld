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


#ifndef FLAGWLD_NET_CHILD_H
#define FLAGWLD_NET_CHILD_H

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

class Child : boost::noncopyable
{
 public:
  Child(const EventLoopPtr& loop, pid_t pid);
  Child(const EventLoopPtr& loop, const ChildCallback& cb, pid_t pid);
  ~Child();

 void setChildCallback(const ChildCallback& cb)
  { callback_ = cb; }
#ifdef __GXX_EXPERIMENTAL_CXX0X__
  Child(const EventLoopPtr& loop, ChildCallback&& cb, pid_t pid);
  void setSingalallback(ChildCallback&& cb)
  { callback_ = std::move(cb); }
#endif

  void start();
  void stop();

  void tie(const boost::shared_ptr<void>& obj);

 private:
  inline static void child_cb (EV_P_ ev_child* handle, int revents)
  {
    Child* child_ptr = static_cast<Child*>(handle->data);
    assert( child_ptr );
    
    child_ptr->handleEvent(handle->rpid, handle->rstatus);
  }
  void startInLoop();
  void stopInLoop();
  void handleEvent(pid_t, int);

  bool running_;
  EventLoopPtr loop_;
  ChildCallback callback_;
  pid_t pid_;
  int status_;
  bool tied_;
  boost::weak_ptr<void> tie_;
  struct ev_child child_watcher_;
};
}
}
#endif  // FLAGWLD_NET_CHILD_H
