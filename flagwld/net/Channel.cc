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


#include <flagwld/base/Logging.h>
#include <flagwld/net/Channel.h>
#include <flagwld/net/EventLoop.h>

#include <boost/bind.hpp>

#include <sstream>

using namespace flagwld;
using namespace flagwld::net;

#pragma GCC diagnostic ignored "-Wold-style-cast"
Channel::Channel(const EventLoopPtr& loop, int fd__)
  : loop_(loop),
    fd_(fd__),
    events_(0),
    revents_(0),
    tied_(false),
    eventHandling_(false),
    addedToLoop_(false)
{
  LOG_DEBUG << "Channel::ctor[-] at " << this
            << " fd=" << fd_;

  io_watcher_.data = this;
  ev_io_init(&io_watcher_, io_cb, fd_, events_);
}
#pragma GCC diagnostic error "-Wold-style-cast"

Channel::~Channel()
{
  LOG_DEBUG << "Channel::dtor[-] at " << this
            << " fd=" << fd_;
  assert(!eventHandling_);
  assert(!addedToLoop_);
}

void Channel::tie(const boost::shared_ptr<void>& obj)
{
  tie_ = obj;
  tied_ = true;
}

void Channel::update()
{
  addedToLoop_ = true;
 if (loop_->isInLoopThread())
 {
   updateInLoop();
 }
 else
 {
   loop_->queueInLoop( boost::bind(&Channel::updateInLoop, this) );
 }  
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
void Channel::updateInLoop()
{
  loop_->assertInLoopThread();

  if (isNoneEvent())
  {
     LOG_DEBUG << "EV_stop Channel at " << this << " fd=" << fd();
    ev_io_stop(loop_->loop_, &io_watcher_);
  }
  else
  {
    LOG_DEBUG << "EV_stop2 Channel at " << this << " fd=" << fd();
    ev_io_stop(loop_->loop_, &io_watcher_);
    ev_io_set(&io_watcher_, fd_, events_);
    LOG_DEBUG << "EV_start Channel at " << this << " fd=" << fd();
    ev_io_start(loop_->loop_, &io_watcher_);
  }
}
#pragma GCC diagnostic error "-Wold-style-cast"

void Channel::remove()
{
  assert(isNoneEvent());

  addedToLoop_ = false;
  if (loop_->isInLoopThread())
  {
    removeInLoop();
  }
  else
  {
    loop_->queueInLoop( boost::bind(&Channel::removeInLoop, this) );
  }
}

void Channel::removeInLoop()
{
  LOG_DEBUG << "EV_stop Channel at " << this << " fd=" << fd();
  loop_->assertInLoopThread();

  ev_io_stop(loop_->loop_, &io_watcher_);
}

void Channel::handleEvent()
{
  boost::shared_ptr<void> guard;
  if (tied_)
  {
    guard = tie_.lock();
    if (guard)
    {
      handleEventWithGuard();
    }
  }
  else
  {
    handleEventWithGuard();
  }
}

void Channel::handleEventWithGuard()
{
  eventHandling_ = true;
  LOG_TRACE << reventsToString() << " at " << this;

  if (revents_ & EV_NONE)
  {
    LOG_WARN << "Channel::handle_event() NONE";
    if (closeCallback_) closeCallback_();
  }
  if (revents_ & EV_ERROR)
  {
    LOG_WARN << "Channel::handle_event() ERROR";
    if (errorCallback_) errorCallback_();
  }
  if (revents_ & EV_READ)
  {
    if (readCallback_) readCallback_();
  }
  if (revents_ & EV_WRITE)
  {
    if (writeCallback_) writeCallback_();
  }
  eventHandling_ = false;
}

string Channel::reventsToString() const
{
  std::ostringstream oss;
  oss << fd_ << ": ";
  if (revents_ & EV_READ)
    oss << "READ ";
  if (revents_ & EV_WRITE)
    oss << "WRITE ";
  if (revents_ & EV_ERROR)
    oss << "ERROR";
  if (revents_ & EV_NONE)
    oss << "NONE";

  return oss.str().c_str();
}
