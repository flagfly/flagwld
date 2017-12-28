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


#ifndef FLAGWLD_NET_SOCKSERVER_H
#define FLAGWLD_NET_SOCKSERVER_H

#include <flagwld/base/Types.h>
#include <flagwld/net/SockConnection.h>

#include <map>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace flagwld
{
namespace net
{

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

typedef boost::shared_ptr<EventLoopThreadPool> EventLoopThreadPoolPtr; 
///
/// Sock server, supports single-threaded and thread-pool models.
///
/// This is an interface class, so don't expose too much details.
class SockServer : boost::noncopyable
{
 public:
  typedef boost::function<void(EventLoopPtr)> ThreadInitCallback;
  typedef boost::function<void(EventLoopPtr)> ThreadExitCallback;
  enum Option
  {
    kNoReusePort,
    kReusePort,
  };

  //SockServer(EventLoopPtr& loop, const SockAddress& listenAddr);
  SockServer(const EventLoopPtr& loop,
            const SockAddress& listenAddr,
            const string& nameArg,
            Option option = kNoReusePort);
  SockServer(const EventLoopThreadPoolPtr& loopPool,
            const SockAddress& listenAddr,
            const string& nameArg,
            Option option = kNoReusePort);
  ~SockServer();  // force out-line dtor, for scoped_ptr members.

  const string& hostport() const { return hostport_; }
  const string& name() const { return name_; }
  EventLoopPtr getLoop() const { return loop_; }

  /// Set the number of threads for handling input.
  ///
  /// Always accepts new connection in loop's thread.
  /// Must be called before @c start
  /// @param numThreads
  /// - 0 means all I/O in loop's thread, no thread will created.
  ///   this is the default value.
  /// - 1 means all I/O in another thread.
  /// - N means a thread pool with N threads, new connections
  ///   are assigned on a round-robin basis.
  void setThreadNum(int numThreads);
  void setThreadCpuAffiniity(bool on);
  void setThreadInitCallback(const ThreadInitCallback& cb)
  { threadInitCallback_ = cb; }
  void setThreadExitCallback(const ThreadExitCallback& cb)
  { threadExitCallback_ = cb; }
  /// valid after calling start()
  boost::shared_ptr<EventLoopThreadPool> threadPool()
  { return threadPool_; }

  /// Starts the server if it's not listenning.
  ///
  /// It's harmless to call it multiple times.
  /// Thread safe.
  void start();

  void stop();

  void resume();

  /// Set connection callback.
  /// Not thread safe.
  void setConnectionCallback(const ConnectionCallback& cb)
  { connectionCallback_ = cb; }

 private:
  /// Not thread safe, but in loop
  void newConnection(int sockfd, const SockAddress& peerAddr);
  /// Thread safe.
  void removeConnection(const SockConnectionPtr& conn);
  /// Not thread safe, but in loop
  void removeConnectionInLoop(const SockConnectionPtr& conn);

  typedef std::map<string, SockConnectionPtr> SockConnectionMap;

  EventLoopPtr loop_;  // the acceptor loop
  const string hostport_;
  const string name_;
  SockAddress listenAddr_;
  Option option_;
  boost::scoped_ptr<Acceptor> acceptor_; // avoid revealing Acceptor
  boost::shared_ptr<EventLoopThreadPool> threadPool_;
  bool sharedPool_;
  ConnectionCallback connectionCallback_;
  size_t highWaterMark_;
  ThreadInitCallback threadInitCallback_;
  ThreadExitCallback threadExitCallback_;
  AtomicInt32 started_;
  // always in loop thread
  int nextConnId_;
  SockConnectionMap connections_;
};

}
}

#endif  // FLAGWLD_NET_SOCKSERVER_H
