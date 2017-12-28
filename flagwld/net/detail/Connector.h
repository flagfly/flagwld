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


#ifndef FLAGWLD_NET_CONNECTOR_H
#define FLAGWLD_NET_CONNECTOR_H

#include <flagwld/base/Atomic.h>
#include <flagwld/base/Timestamp.h>
#include <flagwld/net/SockAddress.h>
#include <flagwld/net/Callbacks.h>
#include <flagwld/net/Timer.h>

#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace flagwld
{
namespace net
{

class Channel;
class EventLoop;
typedef boost::shared_ptr<EventLoop> EventLoopPtr;

class Connector : boost::noncopyable,
                  public boost::enable_shared_from_this<Connector>
{
 public:
  typedef boost::function<void (int sockfd)> NewSockConnectionCallback;
  typedef boost::function<bool (int err)> NewSockConnectionErrorCallback;

  Connector(const EventLoopPtr& loop, const SockAddress& serverAddr);
  ~Connector();

  void setNewSockConnectionCallback(const NewSockConnectionCallback& cb)
  { newSockConnectionCallback_ = cb; }

  void setConnectErrorCallback(const NewSockConnectionErrorCallback& cb)
  { newSockConnectionErrorCallback_ = cb; }

#ifdef __GXX_EXPERIMENTAL_CXX0X__
  void setNewSockConnectionCallback(NewSockConnectionCallback&& cb)
  { newSockConnectionCallback_ = std::move(cb); }

  void setNewSockConnectionErrorCallback(NewSockConnectionErrorCallback&& cb)
  { newSockConnectionErrorCallback_ = std::move(cb); }
#endif

  void setConnectTimeoutMs(int ms)
  { connectTimeoutMs_ = ms; }

  void start();  // can be called in any thread
  void restart();  // must be called in loop thread
  void delayRestart();  // must be called in loop thread
  void stop();  // can be called in any thread

  const SockAddress& serverAddress() const { return serverAddr_; }

 private:
  enum States { kDisconnected, kConnecting, kConnected };
  typedef flagwld::detail::AtomicIntegerT<States> AtomicStates;

  static const int kConnectTimeoutMs = 7*1000;
  static const int kMaxRetryDelayMs = 30*1000;
  static const int kMinRetryDelayMs = 5*1000;
  static const int kInitRetryDelayMs = 500;

  inline void setState(States s) { state_.getAndSet(s); }
  inline States getState() { return state_.get(); }
  inline bool compareAndSetState( States oldState, States newState) { return state_.compareAndSwap(oldState, newState); }

  void startInLoop();
  void stopInLoop();
  void connect();
  void connecting(int sockfd);
  void handleWrite();
  void handleError();
  void handleConnectTimeout();
  void retry(int sockfd);
  int removeAndResetChannel();
  void resetChannel();

  EventLoopPtr loop_;
  SockAddress serverAddr_;
  bool connect_; // atomic
  AtomicStates state_;
  boost::scoped_ptr<Channel> channel_;
  NewSockConnectionCallback newSockConnectionCallback_;
  NewSockConnectionErrorCallback newSockConnectionErrorCallback_;
  int retryDelayMs_;
  int connectTimeoutMs_;
  Timestamp lastConnectedTimestamp_;
  boost::scoped_ptr<Timer> connectTimer_;
  boost::scoped_ptr<Timer> retryTimer_;
  bool retrying_;
};

}
}

#endif  // FLAGWLD_NET_CONNECTOR_H
