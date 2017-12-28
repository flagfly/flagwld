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


#ifndef FLAGWLD_NET_TIMER_H
#define FLAGWLD_NET_TIMER_H

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

class Timer : boost::noncopyable
{
 public:
  Timer(const EventLoopPtr& loop, const TimerCallback& cb, double after, double interval);
  Timer(const EventLoopPtr& loop, const TimerCallback& cb);
  Timer(const EventLoopPtr& loop, double after, double interval);
  ~Timer();

  void setTimerCallback(const TimerCallback& cb)
  { callback_ = cb; }
#ifdef __GXX_EXPERIMENTAL_CXX0X__
  Timer(const EventLoopPtr& loop, TimerCallback&& cb, double after, double interval);
  void setTimerCallback(TimerCallback&& cb)
  { callback_ = std::move(cb); }
#endif

  void set(double after, double interval);
  void start();
  void restart();
  void stop();

  /// Tie this channel to the owner object managed by shared_ptr,
  /// prevent the owner object being destroyed in handleEvent.
  void tie(const boost::shared_ptr<void>&);

 private:
  inline static void timer_cb (EV_P_ ev_timer* handle, int revents)
  {
    assert (revents & EV_TIMER);
    Timer* timer_ptr = static_cast<Timer*>(handle->data);
    assert( timer_ptr );
    timer_ptr->handleEvent();
  }
  void handleEvent();
  void startInLoop();
  void restartInLoop();
  void stopInLoop();

  bool running_;
  EventLoopPtr loop_;
  TimerCallback callback_;
  double after_;
  double interval_;
  bool tied_;
  boost::weak_ptr<void> tie_;
  struct ev_timer timer_watcher_;
};
}
}
#endif  // FLAGWLD_NET_TIMER_H
