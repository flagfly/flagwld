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


#include <flagwld/net/Periodic.h>

#include <flagwld/base/Logging.h>
#include <flagwld/net/EventLoop.h>

#include <boost/bind.hpp>

using namespace flagwld;
using namespace flagwld::net;

#pragma GCC diagnostic ignored "-Wold-style-cast"
Periodic::Periodic(const EventLoopPtr& loop, const PeriodicCallback& cb, double offset, double interval)
  :running_(false),
  loop_(loop),
  callback_(cb),
  offset_(offset),
  interval_(interval),
  tied_(false)
{
  LOG_DEBUG << "Periodic::ctor[-] at " << this
            << " at=" << offset_ << " interval=" << interval_;


  periodic_watcher_.data = this;
  ev_periodic_init(&periodic_watcher_, periodic_cb, offset_, interval_, 0);
}

Periodic::Periodic(const EventLoopPtr& loop, const PeriodicCallback& cb, const RescheduleCallback& rescheduleCb)
  :running_(false),
  loop_(loop),
  callback_(cb),
  rescheduleCallback_(rescheduleCb),
  offset_(0.),
  interval_(0.),
  tied_(false)
{
  LOG_DEBUG << "Periodic::ctor[-] at " << this
            << " at=" << offset_ << " interval=" << interval_;

  periodic_watcher_.data = this;
  ev_periodic_init(&periodic_watcher_, periodic_cb, 0., 0., reschedule_cb);
} 

Periodic::Periodic(const EventLoopPtr& loop, double offset, double interval)
  :running_(false),
  loop_(loop),
  offset_(offset),
  interval_(interval),
  tied_(false)
{
  LOG_DEBUG << "Periodic::ctor[-] at " << this
            << " at=" << offset_ << " interval=" << interval_;

  periodic_watcher_.data = this;
  ev_periodic_init(&periodic_watcher_, periodic_cb, offset_, interval_, 0);
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
Periodic::Periodic(const EventLoopPtr& loop, PeriodicCallback&& cb, double offset, double interval)
  :running_(false),
  loop_(loop),
  callback_(std::move(cb)),
  offset_(offset),
  interval_(interval),
  tied_(false)
{
  LOG_DEBUG << "Periodic::ctor[-] at " << this
            << " at=" << offset_ << " interval=" << interval_;


  periodic_watcher_.data = this;
  ev_periodic_init(&periodic_watcher_, periodic_cb, offset_, interval_, 0);
}

Periodic::Periodic(const EventLoopPtr& loop, PeriodicCallback&& cb, RescheduleCallback&& rescheduleCb)
  :running_(false),
  loop_(loop),
  callback_(std::move(cb)),
  rescheduleCallback_(std::move(rescheduleCb)),
  offset_(0.),
  interval_(0.),
  tied_(false)
{
  LOG_DEBUG << "Periodic::ctor[-] at " << this
            << " at=" << offset_ << " interval=" << interval_;

  periodic_watcher_.data = this;
  ev_periodic_init(&periodic_watcher_, periodic_cb, 0., 0., reschedule_cb);
}
#endif

#pragma GCC diagnostic error "-Wold-style-cast"

Periodic::~Periodic()
{
  LOG_DEBUG << "Periodic::dtor[-] at " << this
            << " at=" << offset_ << " interval=" << interval_;

 assert( !running_ );
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
void Periodic::set(double offset, double interval)
{
  offset_ = offset;
  interval_ = interval;
  ev_periodic_set( &periodic_watcher_, offset_, interval_, 0);
}
#pragma GCC diagnostic error "-Wold-style-cast"

void Periodic::start()
{
 assert( callback_ );

 if (loop_->isInLoopThread())
 {
   startInLoop();
 }
 else
 {
   loop_->queueInLoop( boost::bind(&Periodic::startInLoop, this) );
 }
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
void Periodic::startInLoop()
{
  loop_->assertInLoopThread();

  ev_periodic_start(loop_->loop_, &periodic_watcher_);
  running_=true;
}
#pragma GCC diagnostic error "-Wold-style-cast"

void Periodic::restart()
{
 assert( callback_ );

 if (loop_->isInLoopThread())
 {
   restartInLoop();
 }
 else
 {
   loop_->queueInLoop( boost::bind(&Periodic::restartInLoop, this) );
 }
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
void Periodic::restartInLoop()
{
  assert (running_);

  loop_->assertInLoopThread();

  ev_periodic_again(loop_->loop_, &periodic_watcher_);

}
#pragma GCC diagnostic error "-Wold-style-cast"

void Periodic::stop()
{
 if (loop_->isInLoopThread())
 {
   stopInLoop();
 }
 else
 {
   loop_->queueInLoop( boost::bind(&Periodic::stopInLoop, this) );
 }
}

void Periodic::stopInLoop()
{
  loop_->assertInLoopThread();

  ev_periodic_stop(loop_->loop_, &periodic_watcher_);

  running_=false;
}

void Periodic::tie(const boost::shared_ptr<void>& obj)
{
  tie_ = obj;
  tied_ = true;
}

void Periodic::handleEvent()
{
  if (interval_<=0. && !rescheduleCallback_)
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

double Periodic::reSchedule(double now)
{
  boost::shared_ptr<void> guard;
  double next = 0;

  if (tied_)
  {
    guard = tie_.lock();
    if (guard)
    {
      if (rescheduleCallback_)
      {
        next = rescheduleCallback_(now);
      }
    }
  }
  else
  {
    if (rescheduleCallback_)
    {
      next = rescheduleCallback_(now);
    }
  }

  LOG_DEBUG << "Periodic " << this << "reSchedule " <<  next;

  return next;
}
