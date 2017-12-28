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


#include <flagwld/net/Async.h>

#include <flagwld/base/Logging.h>
#include <flagwld/net/EventLoop.h>

#include <boost/bind.hpp>

using namespace flagwld;
using namespace flagwld::net;

#pragma GCC diagnostic ignored "-Wold-style-cast"
Async::Async(const EventLoopPtr& loop)
  :running_(false),
  loop_(loop),
  tied_(false)
{
  LOG_DEBUG << "Async created " << this << " in thread " << CurrentThread::tid();

  async_watcher_.data = this;
  ev_async_init(&async_watcher_, async_cb);
}
Async::Async(const EventLoopPtr& loop, const AsynCallback& cb)
  :running_(false),
  loop_(loop),
  callback_(cb),
  tied_(false)
{
  LOG_DEBUG << "Async created " << this << " in thread " << CurrentThread::tid();

  async_watcher_.data = this;
  ev_async_init(&async_watcher_, async_cb);
}
#pragma GCC diagnostic error "-Wold-style-cast"

Async::~Async()
{
  LOG_DEBUG << "Async destoryed " << this << " in thread " << CurrentThread::tid();

  assert( !running_ );
}

void Async::start()
{
 if (loop_->isInLoopThread())
 {
   startInLoop();
 }
 else
 {
   loop_->queueInLoop( boost::bind(&Async::startInLoop, this) );
 }
}

void Async::startInLoop()
{
  assert( !running_ );
  loop_->assertInLoopThread();

  ev_async_start(loop_->loop_, &async_watcher_);
  running_=true;
}

void Async::wakeup()
{
  assert( running_ );

  ev_async_send(loop_->loop_, &async_watcher_);
}

void Async::stop()
{
 if (loop_->isInLoopThread())
 {
   stopInLoop();
 }
 else
 {
   loop_->queueInLoop( boost::bind(&Async::stopInLoop, this) );
 }
}

void Async::stopInLoop()
{
  assert( running_ );
  loop_->assertInLoopThread();

  ev_async_stop(loop_->loop_, &async_watcher_);

  running_=false;
}

void Async::tie(const boost::shared_ptr<void>& obj)
{
  tie_ = obj;
  tied_ = true;
}

void Async::handleEvent()
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

