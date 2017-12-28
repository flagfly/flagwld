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


#include <flagwld/net/Timer.h>

#include <flagwld/base/Logging.h>
#include <flagwld/net/EventLoop.h>

#include <boost/bind.hpp>

using namespace flagwld;
using namespace flagwld::net;

#pragma GCC diagnostic ignored "-Wold-style-cast"
Timer::Timer(const EventLoopPtr& loop, const TimerCallback& cb, double after, double interval)
  :running_(false),
  loop_(loop),
  callback_(cb),
  after_(after),
  interval_(interval),
  tied_(false)
{
  LOG_DEBUG << "Timer::ctor[-] at " << this
            << " at=" << after_ << " interval=" << interval_;

  timer_watcher_.data = this;
  ev_timer_init(&timer_watcher_, timer_cb, after_, interval_);
}

Timer::Timer(const EventLoopPtr& loop, const TimerCallback& cb)
  :running_(false),
  loop_(loop),
  callback_(cb),
  after_(0.),
  interval_(0.5),
  tied_(false)
{
  LOG_DEBUG << "Timer::ctor[-] at " << this
            << " at=" << after_ << " interval=" << interval_;

  timer_watcher_.data = this;
  ev_timer_init(&timer_watcher_, timer_cb, after_, interval_);
}

Timer::Timer(const EventLoopPtr& loop, double after, double interval)
  :running_(false),
  loop_(loop),
  after_(after),
  interval_(interval),
  tied_(false)
{
  LOG_DEBUG << "Timer::ctor[-] at " << this
            << " at=" << after_ << " interval=" << interval_;

  timer_watcher_.data = this;
  ev_timer_init(&timer_watcher_, timer_cb, after, interval);
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
Timer::Timer(const EventLoopPtr& loop, TimerCallback&& cb, double after, double interval)
  :running_(false),
  loop_(loop),
  callback_(std::move(cb)),
  after_(after),
  interval_(interval),
  tied_(false)
{
  LOG_DEBUG << "Timer::ctor[-] at " << this
            << " at=" << after_ << " interval=" << interval_;

  timer_watcher_.data = this;
  ev_timer_init(&timer_watcher_, timer_cb, after_, interval_);
}
#endif

#pragma GCC diagnostic error "-Wold-style-cast"

Timer::~Timer()
{
  LOG_DEBUG << "Timer::dtor[-] at " << this
            << " at=" << after_ << " interval=" << interval_;

 assert( !running_ );
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
void Timer::set(double after, double interval)
{
  after_ = after;
  interval_ = interval;
  ev_timer_set( &timer_watcher_, after, interval );
}
#pragma GCC diagnostic error "-Wold-style-cast"

void Timer::start()
{
 assert( callback_ );

 LOG_DEBUG << "Timer " << this <<" start : after["<< after_ << "] interval[" << interval_ <<"]";

 if (loop_->isInLoopThread())
 {
   startInLoop();
 }
 else
 {
   loop_->queueInLoop( boost::bind(&Timer::startInLoop, this) );
 }
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
void Timer::startInLoop()
{
  loop_->assertInLoopThread();

  ev_timer_start(loop_->loop_, &timer_watcher_);
  running_=true;
}
#pragma GCC diagnostic error "-Wold-style-cast"

void Timer::restart()
{
 assert( callback_ );

 if (loop_->isInLoopThread())
 {
   restartInLoop();
 }
 else
 {
   loop_->queueInLoop( boost::bind(&Timer::restartInLoop, this) );
 }
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
void Timer::restartInLoop()
{
  loop_->assertInLoopThread();

  assert (running_);

  ev_timer_again(loop_->loop_, &timer_watcher_);
}
#pragma GCC diagnostic error "-Wold-style-cast"

void Timer::stop()
{
 if (loop_->isInLoopThread())
 {
   stopInLoop();
 }
 else
 {
   loop_->queueInLoop( boost::bind(&Timer::stopInLoop, this) );
 }
}

void Timer::stopInLoop()
{
  loop_->assertInLoopThread();

  ev_timer_stop(loop_->loop_, &timer_watcher_);

  running_=false;
}

void Timer::tie(const boost::shared_ptr<void>& obj)
{
  tie_ = obj;
  tied_ = true;
}

void Timer::handleEvent()
{
  LOG_DEBUG << "Timer " << this <<" Fired: after["<< after_ << "] interval[" << interval_ <<"]";

  if (interval_<=0.)
  {
    running_=false;
  }

  boost::shared_ptr<void> guard;
  if (tied_)
  {
    guard = tie_.lock();
    if (guard)
    {
      if (callback_)
      {
        callback_();
      }
    }
  }
  else
  {
    if (callback_)
    {
      callback_();
    }
  }
}
