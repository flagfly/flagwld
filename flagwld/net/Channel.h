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


#ifndef FLAGWLD_NET_CHANNEL_H
#define FLAGWLD_NET_CHANNEL_H

#include <ev.h>

#include <flagwld/base/Types.h>

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

class Channel : boost::noncopyable
{
 public:
  typedef boost::function<void()> EventCallback;

  Channel(const EventLoopPtr& loop, int fd);
  ~Channel();

  void handleEvent();
  void setReadCallback(const EventCallback& cb)
  { readCallback_ = cb; }
  void setWriteCallback(const EventCallback& cb)
  { writeCallback_ = cb; }
  void setCloseCallback(const EventCallback& cb)
  { closeCallback_ = cb; }
  void setErrorCallback(const EventCallback& cb)
  { errorCallback_ = cb; }
#ifdef __GXX_EXPERIMENTAL_CXX0X__
  void setReadCallback(EventCallback&& cb)
  { readCallback_ = std::move(cb); }
  void setWriteCallback(EventCallback&& cb)
  { writeCallback_ = std::move(cb); }
  void setCloseCallback(EventCallback&& cb)
  { closeCallback_ = std::move(cb); }
  void setErrorCallback(EventCallback&& cb)
  { errorCallback_ = std::move(cb); }
#endif
  /// Tie this channel to the owner object managed by shared_ptr,
  /// prevent the owner object being destroyed in handleEvent.
  void tie(const boost::shared_ptr<void>&);

  inline int fd() const { return fd_; }
  inline int events() const { return events_; }
  inline void set_revents(int revt) { revents_ = revt; } // used by pollers
  // int revents() const { return revents_; }
  inline bool isNoneEvent() const { return events_ == EV_NONE; }

  inline void enableReading() { events_ |= EV_READ; update(); }
  inline void disableReading() { events_ &= ~EV_READ; update(); }
  inline void enableWriting() { events_ |= EV_WRITE; update(); }
  inline void disableWriting() { events_ &= ~EV_WRITE; update(); }
  inline void disableAll() { events_ = EV_NONE; update(); }
  inline bool isWriting() const { return events_ & EV_WRITE; }
  inline bool isReading() const { return events_ & EV_READ; }

  // for debug
  string reventsToString() const;

  inline EventLoopPtr ownerLoop() { return loop_; }
  void remove();

 private:
  inline static void io_cb (EV_P_ ev_io* handle, int revents)
  {
    Channel* channel_ptr = static_cast<Channel*>(handle->data);
    assert( channel_ptr );
    channel_ptr->set_revents(revents);
    channel_ptr->handleEvent();
  }
  void update();
  void updateInLoop();
  void removeInLoop();
  void handleEventWithGuard();

  EventLoopPtr loop_;
  const int  fd_;
  int        events_;
  int        revents_; // it's the received event types of epoll or poll

  boost::weak_ptr<void> tie_;
  bool tied_;
  bool eventHandling_;
  bool addedToLoop_;
  EventCallback readCallback_;
  EventCallback writeCallback_;
  EventCallback closeCallback_;
  EventCallback errorCallback_;
  struct ev_io io_watcher_;
};

}
}
#endif  // FLAGWLD_NET_CHANNEL_H
