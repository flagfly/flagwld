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


#include <flagwld/net/SockServer.h>

#include <flagwld/base/Logging.h>
#include <flagwld/net/EventLoop.h>
#include <flagwld/net/EventLoopThreadPool.h>
#include <flagwld/net/SocketsOps.h>
#include <flagwld/net/detail/Acceptor.h>

#include <boost/bind.hpp>
#include <boost/make_shared.hpp> 

#include <stdio.h>  // snprintf

using namespace flagwld;
using namespace flagwld::net;

SockServer::SockServer(const EventLoopPtr& loop,
                     const SockAddress& listenAddr,
                     const string& nameArg,
                     Option option)
  : loop_(CHECK_NOTNULL(get_pointer(loop))?loop:loop),
    hostport_(listenAddr.toString()),
    name_(nameArg),
    listenAddr_(listenAddr),
    option_(option),
    acceptor_(new Acceptor(loop_, listenAddr_, option_ == kReusePort)),
    threadPool_(new EventLoopThreadPool(loop, name_)),
    sharedPool_(false),
    connectionCallback_(defaultConnectionCallback),
    started_(0),
    nextConnId_(1)
{
  acceptor_->setNewConnectionCallback(
      boost::bind(&SockServer::newConnection, this, _1, _2));
}

SockServer::SockServer(const EventLoopThreadPoolPtr& loopPool,
                     const SockAddress& listenAddr,
                     const string& nameArg,
                     Option option)
  : loop_(CHECK_NOTNULL(get_pointer(loopPool))?loopPool->getLoop():loopPool->getLoop()),
    hostport_(listenAddr.toString()),
    name_(nameArg),
    listenAddr_(listenAddr),
    option_(option),
    acceptor_(new Acceptor(loop_, listenAddr_, option_ == kReusePort)),
    threadPool_(loopPool),
    sharedPool_(true),
    connectionCallback_(defaultConnectionCallback),
    started_(0),
    nextConnId_(1)
{
  acceptor_->setNewConnectionCallback(
      boost::bind(&SockServer::newConnection, this, _1, _2));
}

SockServer::~SockServer()
{
  loop_->assertInLoopThread();
  LOG_TRACE << "SockServer::~SockServer [" << name_ << "] destructing";

  for (SockConnectionMap::iterator it(connections_.begin());
      it != connections_.end(); ++it)
  {
    SockConnectionPtr conn(it->second);
    it->second.reset();
    conn->getLoop()->runInLoop(
      boost::bind(&SockConnection::connectDestroyed, conn));
  }
}

void SockServer::setThreadNum(int numThreads)
{
  assert(0 <= numThreads);
  if (!sharedPool_)
  {
    threadPool_->setThreadNum(numThreads);
  }
}

void SockServer::setThreadCpuAffiniity(bool on)
{
  if (!sharedPool_)
  {
    threadPool_->setThreadCpuAffinity(on);
  }
}

void SockServer::start()
{
  LOG_INFO << "SockServer::start [" << name_ << "@" << hostport_ << "] starting";
  if (started_.getAndSet(1) == 0 && !sharedPool_)
  {
    threadPool_->setThreadInitCallback(threadInitCallback_);
    threadPool_->setThreadExitCallback(threadExitCallback_);

    threadPool_->start();
  }

  if (!acceptor_->listenning())
  {
    assert(!acceptor_->listenning());
    loop_->runInLoop(
        boost::bind(&Acceptor::listen, get_pointer(acceptor_)));
  }
}

void SockServer::stop()
{
  LOG_INFO << "SockServer::stop [" << name_ << "@" << hostport_ << "] stopping";
  if (acceptor_->listenning())
  {
    acceptor_.reset();
  }
}

void SockServer::resume()
{
  LOG_INFO << "SockServer::resume [" << name_ << "@" << hostport_ << "] resuming";

  assert (started_.get() == 1);

  assert (!acceptor_);

  if (!acceptor_)
  {
    acceptor_.reset(new Acceptor(loop_, listenAddr_, option_ == kReusePort));
    acceptor_->setNewConnectionCallback(
       boost::bind(&SockServer::newConnection, this, _1, _2));
  }

  assert(!acceptor_->listenning());

  if (!acceptor_->listenning())
  {
    loop_->runInLoop(
      boost::bind(&Acceptor::listen, get_pointer(acceptor_)));
  }
}

void SockServer::newConnection(int sockfd, const SockAddress& peerAddr)
{
  loop_->assertInLoopThread();
  EventLoopPtr ioLoop = threadPool_->getNextLoop();
  char buf[32];
  snprintf(buf, sizeof buf, ":%s#%d", hostport_.c_str(), nextConnId_);
  ++nextConnId_;
  string connName = name_ + buf;

  LOG_TRACE << "SockServer::newSockConnection [" << name_
           << "] - new connection [" << connName
           << "] from " << peerAddr.toString();

  struct sockaddr_un local = sockets::getLocalAddr(sockfd);
  SockAddress localAddr(sockets::sockaddr_cast(&local));
  SockConnectionPtr conn = boost::make_shared<SockConnection>(ioLoop,
                                          connName,
                                          sockfd,
                                          localAddr,
                                          peerAddr);
  connections_[connName] = conn;
  conn->setConnectionIdleCallback(defaultConnectionIdleCallback);
  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(defaultMessageCallback);
  conn->setCloseCallback(
      boost::bind(&SockServer::removeConnection, this, _1)); // FIXME: unsafe
  ioLoop->runInLoop(boost::bind(&SockConnection::connectEstablished, conn));
}

void SockServer::removeConnection(const SockConnectionPtr& conn)
{
 if (loop_->isInLoopThread())
 {
   removeConnectionInLoop(conn);
 }
 else
 {
   loop_->queueInLoop(boost::bind(&SockServer::removeConnectionInLoop, this, conn));
 }
}

void SockServer::removeConnectionInLoop(const SockConnectionPtr& conn)
{
  loop_->assertInLoopThread();
  LOG_TRACE << "SockServer::removeConnectionInLoop [" << name_
           << "] - connection [" << conn->name()
           << "] from " << conn->peerAddress().toString();

  size_t n = connections_.erase(conn->name());
  (void)n;
  assert(n == 1);
  EventLoopPtr ioLoop = conn->getLoop();
  ioLoop->queueInLoop(
      boost::bind(&SockConnection::connectDestroyed, conn));
}
