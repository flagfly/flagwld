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


#include <flagwld/net/SockClient.h>

#include <flagwld/base/Logging.h>
#include <flagwld/net/EventLoop.h>
#include <flagwld/net/SocketsOps.h>
#include <flagwld/net/detail/Connector.h>

#include <boost/bind.hpp>
#include <boost/make_shared.hpp>

#include <stdio.h>  // snprintf
#include <errno.h>  // snprintf

using namespace flagwld;
using namespace flagwld::net;

// SockClient::SockClient(const EventLoopPtr& loop)
//   : loop_(loop)
// {
// }

// SockClient::SockClient(const EventLoopPtr& loop, const string& host, uint16_t port)
//   : loop_(CHECK_NOTNULL(loop)),
//     serverAddr_(host, port)
// {
// }

namespace flagwld
{
namespace net
{
namespace detail
{

void removeSockConnection(const EventLoopPtr& loop, const SockConnectionPtr& conn)
{
  loop->queueInLoop(boost::bind(&SockConnection::connectDestroyed, conn));
}

void removeConnector(const ConnectorPtr& connector)
{
  //connector->
}

}
}
}

SockClient::SockClient(const EventLoopPtr& loop,
                     const SockAddress& serverAddr,
                     const string& nameArg)
  : loop_(CHECK_NOTNULL(get_pointer(loop))?loop:loop),
    connector_(new Connector(loop, serverAddr)),
    hostport_(serverAddr.toString()),
    serverAddr_(serverAddr),
    name_(nameArg),
    connectionCallback_(defaultConnectionCallback),
    connectErrorCallback_(defaultConnectErrorCallback),
    connecttimeout_(0.),
    retry_(false),
    delayRetry_(false),
    connect_(true),
    nextConnId_(1)
{
  LOG_DEBUG << "SockClient::ctor[" << name_
           << "] at " << this << " - connector " << get_pointer(connector_);
  connector_->setNewSockConnectionCallback(
      boost::bind(&SockClient::newConnection, this, _1));
  connector_->setConnectErrorCallback(
      boost::bind(&SockClient::newConnectionError, this, _1));
}

SockClient::~SockClient()
{
  LOG_DEBUG << "SockClient::dtor[" << name_
           << "] at " << this <<" - connector " << get_pointer(connector_);
  SockConnectionPtr conn;
  bool unique = false;
  {
    MutexLockGuard lock(mutex_);
    unique = connection_.unique();
    conn = connection_;
  }
  if (conn)
  {
    assert(loop_ == conn->getLoop());
    // FIXME: not 100% safe, if we are in different thread
    CloseCallback cb = boost::bind(&detail::removeSockConnection, loop_, _1);
    loop_->runInLoop(
        boost::bind(&SockConnection::setCloseCallback, conn, cb));
    if (unique)
    {
      //detail::removeSockConnection(loop_, conn);
      conn->forceClose();
    }
  }
  else
  {
    connector_->stop();
    // FIXME: HACK
  /* XXXXXXXXXXXXXXXXX for futer **********/
    loop_->queueInLoop(boost::bind(&detail::removeConnector, connector_));
  }
}

void SockClient::connect()
{
  // FIXME: check state
  LOG_TRACE << "SockClient::connect[" << name_ << "] at " << this << " - connecting to " << hostport_;

  if (connecttimeout_>0)
  {
    connector_->setConnectTimeoutMs(static_cast<int>(connecttimeout_*1000));
  }
  connect_ = true;
  connector_->start();
}

void SockClient::disconnect()
{
  LOG_TRACE << "SockClient::disconnect[" << name_ << "] at " << this << " - disconnecting from " << hostport_;

  connect_ = false;

  {
    MutexLockGuard lock(mutex_);
    if (connection_)
    {
      connection_->shutdown();
    }
  }
}
void SockClient::stop()
{
  LOG_TRACE << "SockClient::stop[" << name_ << "] at " << this << " - stopping from " << hostport_;

  connect_ = false;
  connector_->stop();
}


void SockClient::newConnection(int sockfd)
{
  loop_->assertInLoopThread();

  struct sockaddr_un peer = sockets::getPeerAddr(sockfd);
  SockAddress peerAddr(sockets::sockaddr_cast(&peer));

  char buf[32];
  snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toString().c_str(), nextConnId_);
  ++nextConnId_;
  string connName = name_ + buf;

  struct sockaddr_un local = sockets::getLocalAddr(sockfd);
  SockAddress localAddr(sockets::sockaddr_cast(&local));

  SockConnectionPtr conn = boost::make_shared<SockConnection>(loop_,
                                          connName,
                                          sockfd,
                                          localAddr,
                                          peerAddr);

  LOG_TRACE << "SockClient::newSockConnection[" << name_ << "] at " << this << " - connected to " << hostport_ << " - conn at " << get_pointer(conn);
  conn->setConnectionIdleCallback(defaultConnectionIdleCallback);
  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(defaultMessageCallback);
  conn->setCloseCallback(
      boost::bind(&SockClient::removeConnection, this, _1)); // FIXME: unsafe
  {
    MutexLockGuard lock(mutex_);
    connection_ = conn;
  }
  conn->connectEstablished();
}

void SockClient::removeConnection(const SockConnectionPtr& conn)
{
  LOG_TRACE << "SockClient::removeConnection[" << name_ << "] at " << this << " - disconnecting from " << hostport_;
  loop_->assertInLoopThread();
  assert(loop_ == conn->getLoop());

  {
    MutexLockGuard lock(mutex_);
    assert(connection_ == conn);
    connection_.reset();
  }

  loop_->queueInLoop(boost::bind(&SockConnection::connectDestroyed, conn));
  if (connect_)
  {
    if (retry_)
    {
      LOG_TRACE << "SockClient::connect[" << name_ << "] - Reconnecting to "
             << connector_->serverAddress().toString();
      connector_->restart();
    }
    else if (delayRetry_)
    {
      LOG_TRACE << "SockClient::connect[" << name_ << "] - Delay Reconnecting to "
             << connector_->serverAddress().toString();
      connector_->delayRestart();
    }
  }
}

bool SockClient::newConnectionError(int err)
{
  LOG_TRACE << "SockClient::newConnectionError[" << name_ << "] at " << this << " - error: " << strerror(err);

  bool keepretry = connectErrorCallback_(serverAddr_);

  return keepretry;
}

