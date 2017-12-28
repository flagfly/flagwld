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


#ifndef FLAGWLD_NET_SOCKCONNECTION_H
#define FLAGWLD_NET_SOCKCONNECTION_H

#include <flagwld/base/StringPiece.h>
#include <flagwld/base/Types.h>
#include <flagwld/base/Atomic.h>
#include <flagwld/net/Callbacks.h>
#include <flagwld/net/Buffer.h>
#include <flagwld/net/SockAddress.h>
#include <flagwld/net/Timer.h>

#include <boost/any.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

// struct tcp_info is in <netinet/tcp.h>
struct tcp_info;

namespace flagwld
{
namespace net
{

class Channel;
class EventLoop;
typedef boost::shared_ptr<EventLoop> EventLoopPtr;
class Socket;

///
/// TCP connection, for both client and server usage.
///
/// This is an interface class, so don't expose too much details.
///
/// idlecb; highwather cb;write complete cb should be use at tcpconn directly
///
class SockConnection : boost::noncopyable,
                      public boost::enable_shared_from_this<SockConnection>
{
 public:
  /// Constructs a SockConnection with a connected sockfd
  ///
  /// User should not create this object.
  SockConnection(const EventLoopPtr& loop,
                const string& name,
                int sockfd,
                const SockAddress& localAddr,
                const SockAddress& peerAddr);
  ~SockConnection();

  inline EventLoopPtr getLoop() const { return loop_; }
  inline const string& name() const { return name_; }
  inline const SockAddress& localAddress() const { return localAddr_; }
  inline const SockAddress& peerAddress() const { return peerAddr_; }
  inline bool connected() { return state_.get() == kConnected; }
  inline bool disconnected() { return state_.get() == kDisconnected; }
  // return true if success.

  string getInfoString() const;
  string getTcpInfoString () const __attribute__((deprecated)) { return getInfoString(); } 

  // void send(string&& message); // C++11
  void send(const void* message, int len);
  void send(const StringPiece& message);
  // void send(Buffer&& message); // C++11
  void send(Buffer* message);  // this one will swap data
  void shutdown(); // NOT thread safe, no simultaneous calling
  //void shutdownAndForceCloseAfter(double seconds); // NOT thread safe, no simultaneous calling
  void forceClose();
  //void forceCloseWithDelay(double seconds);
  void setTcpNoDelay(bool on);
  void setSndBuffer(int buf_len);
  void setRcvBuffer(int buf_len);
  // reading or not
  void startRead();
  void stopRead();
  bool isReading() const { return reading_; }; // NOT thread safe, may race with start/stopReadInLoop

  void setBulksize(size_t n) { bulksize_ = n; }

  void enableIdleTimer( double tmo );
  void disableIdleTimer();
  inline double idletime() { return idleTimeout_; }

  inline void setContext(const boost::any& context)
  { context_ = context; }

  inline const boost::any& getContext() const
  { return context_; }

  inline boost::any* getMutableContext()
  { return &context_; }

  inline void setConnectionIdleCallback(const ConnectionIdleCallback& cb)
  { idleCallback_ = cb; }

  inline void setConnectionCallback(const ConnectionCallback& cb)
  { connectionCallback_ = cb; }

  inline void setMessageCallback(const MessageCallback& cb)
  { messageCallback_ = cb; }

  inline void setWriteCompleteCallback(const WriteCompleteCallback& cb)
  { writeCompleteCallback_ = cb; }

  inline void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark=64*1024*1024)
  { highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark; }

  /// Advanced interface
  inline Buffer* inputBuffer()
  { return &inputBuffer_; }

  inline Buffer* outputBuffer()
  { return &outputBuffer_; }

  /// Internal use only.
  inline void setCloseCallback(const CloseCallback& cb)
  { closeCallback_ = cb; }

  // called when Server accepts a new connection
  void connectEstablished();   // should be called only once
  // called when Server has removed me from its map
  void connectDestroyed();  // should be called only once

 private:
  enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
  typedef flagwld::detail::AtomicIntegerT<StateE> AtomicStateE;

  void handleRead();
  void handleWrite();
  void handleClose();
  void handleError();
  void handleIdle();
  void enableIdleTimerInLoop(double tmo);
  void disableIdleTimerInLoop();

  //void sendInLoop(string&& message);
  void sendInLoop(const StringPiece& message);
  void sendInLoop(const void* message, size_t len);
  void shutdownInLoop();
  //void shutdownAndForceCloseInLoop(double seconds);
  void forceCloseInLoop();
  inline void setState(StateE s) { state_.getAndSet(s); }
  inline StateE getState() { return state_.get(); }
  inline bool compareAndSetState( StateE oldState, StateE newState) { return state_.compareAndSwap(oldState, newState); }
  const char* stateToString();

  void startReadInLoop();
  void stopReadInLoop();

  EventLoopPtr loop_;
  const string name_;
  AtomicStateE state_;
  bool reading_;
  // we don't expose those classes to client.
  boost::scoped_ptr<Socket> socket_;
  boost::scoped_ptr<Channel> channel_;
  const SockAddress localAddr_;
  const SockAddress peerAddr_;
  ConnectionIdleCallback idleCallback_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  HighWaterMarkCallback highWaterMarkCallback_;
  CloseCallback closeCallback_;
  size_t highWaterMark_;
  bool prevHighWaterMark_;
  Buffer inputBuffer_;
  Buffer outputBuffer_; // FIXME: use list<Buffer> as output buffer.
  boost::any context_;
  // FIXME: creationTime_, lastReceiveTime_
  //        bytesReceived_, bytesSent_
  double idleTimeout_;
  boost::scoped_ptr<Timer> idletimer_;// run every idleTimeout_ 
  bool idleTiming_;
  size_t bulksize_;
};

typedef boost::shared_ptr<SockConnection> SockConnectionPtr;

}
}

#endif  // FLAGWLD_NET_SOCKCONNECTION_H
