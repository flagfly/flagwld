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


#include <flagwld/net/Child.h>

#include <flagwld/base/Logging.h>
#include <flagwld/net/EventLoop.h>

#include <boost/bind.hpp>

using namespace flagwld;
using namespace flagwld::net;

#pragma GCC diagnostic ignored "-Wold-style-cast"
Child::Child(const EventLoopPtr& loop, pid_t pid)
  :running_(false),
  loop_(loop),
  pid_(pid),
  tied_(false)
{
  LOG_DEBUG << "Child created " << this << " in thread " << CurrentThread::tid();

  child_watcher_.data = this;
  ev_child_init(&child_watcher_, child_cb, pid_, 0);
}
Child::Child(const EventLoopPtr& loop, const ChildCallback& cb, int pid)
  :running_(false),
  loop_(loop),
  callback_(cb),
  pid_(pid),
  tied_(false)
{
  LOG_DEBUG << "Child created " << this << " in thread " << CurrentThread::tid();

  child_watcher_.data = this;
  ev_child_init(&child_watcher_, child_cb, pid_, 0);
}
#ifdef __GXX_EXPERIMENTAL_CXX0X__
Child::Child(const EventLoopPtr& loop, ChildCallback&& cb, int pid)
  :running_(false),
  loop_(loop),
  callback_(std::move(cb)),
  pid_(pid),
  tied_(false)
{
  LOG_DEBUG << "Child created " << this << " in thread " << CurrentThread::tid();

  child_watcher_.data = this;
  ev_child_init(&child_watcher_, child_cb, pid_, 0);
}
#endif
#pragma GCC diagnostic error "-Wold-style-cast"

Child::~Child()
{
  LOG_DEBUG << "Child destoryed " << this << " in thread " << CurrentThread::tid();
  assert( !running_ );
}

void Child::start()
{
 if (loop_->isInLoopThread())
 {
   startInLoop();
 }
 else
 {
   loop_->queueInLoop( boost::bind(&Child::startInLoop, this) );
 }
}

void Child::startInLoop()
{
  assert( !running_ );
  loop_->assertInLoopThread();

  ev_child_start(loop_->loop_, &child_watcher_);
  running_=true;
}

void Child::stop()
{
 if (loop_->isInLoopThread())
 {
   stopInLoop();
 }
 else
 {
   loop_->queueInLoop( boost::bind(&Child::stopInLoop, this) );
 }
}

void Child::stopInLoop()
{
  assert( running_ );
  loop_->assertInLoopThread();

  ev_child_stop(loop_->loop_, &child_watcher_);

  running_=false;
}

void Child::tie(const boost::shared_ptr<void>& obj)
{
  tie_ = obj;
  tied_ = true;
}

void Child::handleEvent(pid_t pid, int status)
{
  LOG_DEBUG << "Watch Child: " << pid_ << " Catch Child: " << pid;

  boost::shared_ptr<void> guard;
  if (tied_)
  {
    guard = tie_.lock();
    if (guard)
    {
      if (callback_)
      {
        callback_(pid, status);
      }
    }
  }
  else
  {
    if (callback_)
    {
      callback_(pid, status);
    }
  }
}

