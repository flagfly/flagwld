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


#include <flagwld/net/Signal.h>

#include <flagwld/base/Logging.h>
#include <flagwld/net/EventLoop.h>

#include <boost/bind.hpp>

using namespace flagwld;
using namespace flagwld::net;

#pragma GCC diagnostic ignored "-Wold-style-cast"
Signal::Signal(const EventLoopPtr& loop, int signum)
  :running_(false),
  loop_(loop),
  signum_(signum),
  tied_(false)
{
  LOG_DEBUG << "Signal created " << this << " in thread " << CurrentThread::tid();

  signal_watcher_.data = this;
  ev_signal_init(&signal_watcher_, signal_cb, signum_);
}
Signal::Signal(const EventLoopPtr& loop, const SignalCallback& cb, int signum)
  :running_(false),
  loop_(loop),
  callback_(cb),
  signum_(signum),
  tied_(false)
{
  LOG_DEBUG << "Signal created " << this << " in thread " << CurrentThread::tid();

  signal_watcher_.data = this;
  ev_signal_init(&signal_watcher_, signal_cb, signum_);
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
Signal::Signal(const EventLoopPtr& loop, SignalCallback&& cb, int signum)
  :running_(false),
  loop_(loop),
  callback_(std::move(cb)),
  signum_(signum),
  tied_(false)
{
  LOG_DEBUG << "Signal created " << this << " in thread " << CurrentThread::tid();

  signal_watcher_.data = this;
  ev_signal_init(&signal_watcher_, signal_cb, signum_);
}
#endif
#pragma GCC diagnostic error "-Wold-style-cast"

Signal::~Signal()
{
  LOG_DEBUG << "Signal destoryed " << this << " in thread " << CurrentThread::tid();
  assert( !running_ );
}

void Signal::start()
{
 if (loop_->isInLoopThread())
 {
   startInLoop();
 }
 else
 {
   loop_->queueInLoop( boost::bind(&Signal::startInLoop, this) );
 }
}

void Signal::startInLoop()
{
  assert( !running_ );
  loop_->assertInLoopThread();

  ev_signal_start(loop_->loop_, &signal_watcher_);
  running_=true;
}

void Signal::stop()
{
 if (loop_->isInLoopThread())
 {
   stopInLoop();
 }
 else
 {
   loop_->queueInLoop( boost::bind(&Signal::stopInLoop, this) );
 }
}

void Signal::stopInLoop()
{
  assert( running_ );
  loop_->assertInLoopThread();

  ev_signal_stop(loop_->loop_, &signal_watcher_);

  running_=false;
}

void Signal::tie(const boost::shared_ptr<void>& obj)
{
  tie_ = obj;
  tied_ = true;
}

void Signal::handleEvent()
{
  LOG_DEBUG << "Catch Signal: " << signum_;

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

