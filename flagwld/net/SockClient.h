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


#ifndef FLAGWLD_NET_SOCKCLIENT_H
#define FLAGWLD_NET_SOCKCLIENT_H

#include <boost/noncopyable.hpp>

#include <flagwld/base/Mutex.h>
#include <flagwld/net/SockConnection.h>

namespace flagwld
{
namespace net
{

class Connector;
typedef boost::shared_ptr<Connector> ConnectorPtr;

class SockClient : boost::noncopyable
{
 public:
  // SockClient(const EventLoopPtr& loop);
  // SockClient(const EventLoopPtr& loop, const string& host, uint16_t port);
  SockClient(const EventLoopPtr& loop,
            const SockAddress& serverAddr,
            const string& nameArg);
  ~SockClient();  // force out-line dtor, for scoped_ptr members.

  void connect();
  void disconnect();
  void stop();

  SockConnectionPtr connection() const
  {
    MutexLockGuard lock(mutex_);
    return connection_;
  }

  const string& hostport() const
  { return hostport_; }

  const string& name() const
  { return name_; }

  EventLoopPtr getLoop() const { return loop_; }

  void enableRetry() { retry_ = true; delayRetry_=false;}
  void enableDelayRetry() { delayRetry_ = true; retry_ = false; }
  void disableRetry() { retry_ = false; delayRetry_=false; }
  bool retry() const { return retry_; }
  bool delayRetry() const { return delayRetry_; }

  void enableConnectTimer(double tmo)
  { connecttimeout_ = tmo; }

  /// Set connection callback.
  /// Not thread safe.
  void setConnectionCallback(const ConnectionCallback& cb)
  { connectionCallback_ = cb; }

  void setConnectErrorCallback(const ConnectErrorCallback& cb)
  { connectErrorCallback_ = cb; }

#ifdef __GXX_EXPERIMENTAL_CXX0X__
  void setConnectionCallback(ConnectionCallback&& cb)
  { connectionCallback_ = std::move(cb); }
  void setConnectErrorCallback(ConnectErrorCallback&& cb)
  { connectErrorCallback_ = std::move(cb); }
#endif

 private:
  /// Not thread safe, but in loop
  void newConnection(int sockfd);
  /// Not thread safe, but in loop
  bool newConnectionError(int err);
  /// Not thread safe, but in loop
  void removeConnection(const SockConnectionPtr& conn);

  EventLoopPtr loop_;
  ConnectorPtr connector_; // avoid revealing Connector
  const string hostport_;
  SockAddress serverAddr_;
  const string name_;
  ConnectionCallback connectionCallback_;
  ConnectErrorCallback connectErrorCallback_;
  double connecttimeout_;
  bool retry_;   // atomic
  bool delayRetry_;   // atomic
  bool connect_; // atomic
  // always in loop thread
  int nextConnId_;
  mutable MutexLock mutex_;
  SockConnectionPtr connection_; // @GuardedBy mutex_
};

}
}

#endif  // FLAGWLD_NET_SOCKCLIENT_H
