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


#include <flagwld/net/Idle.h>

#include <flagwld/base/Logging.h>
#include <flagwld/net/EventLoop.h>

#include <boost/bind.hpp>

using namespace flagwld;
using namespace flagwld::net;

#pragma GCC diagnostic ignored "-Wold-style-cast"
Idle::Idle(const EventLoopPtr& loop)
  :running_(false),
  loop_(loop),
  tied_(false)
{
  LOG_DEBUG << "Idle created " << this << " in thread " << CurrentThread::tid();

  idle_watcher_.data = this;
  ev_idle_init(&idle_watcher_, idle_cb);
}
Idle::Idle(const EventLoopPtr& loop, const IdleCallback& cb)
  :running_(false),
  loop_(loop),
  callback_(cb),
  tied_(false)
{
  LOG_DEBUG << "Idle created " << this << " in thread " << CurrentThread::tid();

  idle_watcher_.data = this;
  ev_idle_init(&idle_watcher_, idle_cb);
}
#pragma GCC diagnostic error "-Wold-style-cast"

Idle::~Idle()
{
  LOG_DEBUG << "Idle destoryed " << this << " in thread " << CurrentThread::tid();

  assert( !running_ );
}

void Idle::start()
{
 if (loop_->isInLoopThread())
 {
   startInLoop();
 }
 else
 {
   loop_->queueInLoop( boost::bind(&Idle::startInLoop, this) );
 }
}

void Idle::startInLoop()
{
  assert( !running_ );
  loop_->assertInLoopThread();

  ev_idle_start(loop_->loop_, &idle_watcher_);
  running_=true;
}

void Idle::stop()
{
 if (loop_->isInLoopThread())
 {
   stopInLoop();
 }
 else
 {
   loop_->queueInLoop( boost::bind(&Idle::stopInLoop, this) );
 }
}

void Idle::stopInLoop()
{
  assert( running_ );
  loop_->assertInLoopThread();

  ev_idle_stop(loop_->loop_, &idle_watcher_);

  running_=false;
}

void Idle::tie(const boost::shared_ptr<void>& obj)
{
  tie_ = obj;
  tied_ = true;
}

void Idle::handleEvent()
{
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

