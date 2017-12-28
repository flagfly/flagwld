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


#include <flagwld/net/detail/Connector.h>

#include <flagwld/base/Logging.h>
#include <flagwld/net/Channel.h>
#include <flagwld/net/EventLoop.h>
#include <flagwld/net/SocketsOps.h>

#include <boost/bind.hpp>
#include <boost/random.hpp>

#include <errno.h>

using namespace flagwld;
using namespace flagwld::net;

const int Connector::kConnectTimeoutMs;
const int Connector::kInitRetryDelayMs;
const int Connector::kMaxRetryDelayMs;

Connector::Connector(const EventLoopPtr& loop, const SockAddress& serverAddr)
  : loop_(loop),
    serverAddr_(serverAddr),
    connect_(false),
    state_(kDisconnected),
    retryDelayMs_(kInitRetryDelayMs),
    connectTimeoutMs_(kConnectTimeoutMs),
    retrying_(false)
{
  retryTimer_.reset(new Timer(loop_, boost::bind(&Connector::startInLoop, this), retryDelayMs_/1000.0, 0.));
  connectTimer_.reset(new Timer(loop_, boost::bind(&Connector::handleConnectTimeout, this), connectTimeoutMs_/1000.0, 0.));
  LOG_DEBUG << "Connector::ctor[" <<  serverAddr_.toString() << "] at " << this << " - timer at " << get_pointer(retryTimer_);
}

Connector::~Connector()
{
  LOG_DEBUG << "Connector::dtor[" <<  serverAddr_.toString() << "] at " << this;
  assert(!channel_);
}

void Connector::start()
{
  LOG_TRACE << "Connector::start at " << this << " " << serverAddr_.toString();
  if (loop_->isInLoopThread())
  {
    startInLoop();
  }
  else
  {
    loop_->queueInLoop(boost::bind(&Connector::startInLoop, shared_from_this())); // FIXME: unsafe
  }
}

void Connector::startInLoop()
{
  LOG_TRACE << "Connector::startInLoop at " << this << " " << serverAddr_.toString();
  loop_->assertInLoopThread();
  assert(getState() == kDisconnected);

  connect_ = true;

  if (retrying_)
  {
    retrying_ = false;
  }

  connect();
}

void Connector::stop()
{
  LOG_TRACE << "Connector::stop at " << this << " " << serverAddr_.toString();
  if (loop_->isInLoopThread())
  {
    stopInLoop();
  }
  else
  {
    loop_->queueInLoop(boost::bind(&Connector::stopInLoop, shared_from_this())); // FIXME: unsafe
  }
}

void Connector::stopInLoop()
{
  LOG_TRACE << "Connector::stopInLoop at " << this << " " << serverAddr_.toString();
  loop_->assertInLoopThread();

  connect_ = false;

  if (compareAndSetState(kConnecting, kDisconnected))
  {
    connectTimer_->stop();
    int sockfd = removeAndResetChannel();
    sockets::close(sockfd);
  }
  else if (getState() == kConnected)
  {
    setState(kDisconnected);
  }
  else if (getState() == kDisconnected)
  {
    if (retrying_)
    {
      retrying_=false;
      retryTimer_->stop();
    }
  }
}

void Connector::connect()
{
  LOG_TRACE << "Connector::connect at " << this << " " << serverAddr_.toString();

  int sockfd = sockets::createStreamNonblockingOrDie(serverAddr_.family());

  int ret = sockets::connect(sockfd, serverAddr_.getSockAddr(), serverAddr_.getSockLen());
  int savedErrno = (ret == 0) ? 0 : errno;
  switch (savedErrno)
  {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
      connecting(sockfd);
      break;

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
      if (!newSockConnectionErrorCallback_(savedErrno))
      {
        connect_ = false;
      }
      retry(sockfd);
      break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
      LOG_SYSERR << "connect error in Connector::startInLoop " << savedErrno << " " << serverAddr_.toString();
      if (!newSockConnectionErrorCallback_(savedErrno))
      {
        connect_ = false;
      }
      retry(sockfd);
      break;

    default:
      LOG_SYSERR << "Unexpected error in Connector::startInLoop " << savedErrno << " " << serverAddr_.toString();
      if (!newSockConnectionErrorCallback_(savedErrno))
      {
        connect_ = false;
      }
      retry(sockfd);
      break;
  }
}

void Connector::restart()
{ 
  LOG_TRACE << "Connector::restart at " << this << " " << serverAddr_.toString();
  loop_->assertInLoopThread();
  assert(!retrying_);

  setState(kDisconnected);
    
  retryDelayMs_ = kInitRetryDelayMs;
  startInLoop();
}

void Connector::delayRestart()
{
  LOG_TRACE << "Connector::delayRestart at " << this << " " << serverAddr_.toString();
  loop_->assertInLoopThread();
  assert(!retrying_);

  setState(kDisconnected);

  Timestamp now = Timestamp::now();

  if ((timeDifference(now, lastConnectedTimestamp_)*1000.0)>kMaxRetryDelayMs)
  {
    retryDelayMs_ = kInitRetryDelayMs;
    startInLoop();
  }
  else
  {
    retryTimer_->set(retryDelayMs_/1000.0, 0.);
    retryTimer_->start();
    retrying_ = true;
    if (retryDelayMs_>kMinRetryDelayMs)
    {
      boost::mt19937 gen( static_cast<uint32_t>(now.microSecondsSinceEpoch()%Timestamp::kMicroSecondsPerSecond) );
      boost::uniform_int<> dist(kMinRetryDelayMs, kMaxRetryDelayMs);
      boost::variate_generator<boost::mt19937&, boost::uniform_int<> > die(gen,dist);
      retryDelayMs_ = die();
    }
    else
    {
      retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
    }
  }
}

void Connector::connecting(int sockfd)
{
  LOG_TRACE << "Connector::connecting at " << this << " - channel at " << get_pointer(channel_) << " " << serverAddr_.toString();

  setState(kConnecting);
  assert(!channel_);
  channel_.reset(new Channel(loop_, sockfd));
  channel_->setWriteCallback(
      boost::bind(&Connector::handleWrite, shared_from_this())); // FIXME: unsafe
  channel_->setErrorCallback(
      boost::bind(&Connector::handleError, shared_from_this())); // FIXME: unsafe

  // channel_->tie(shared_from_this()); is not working,
  // as channel_ is not managed by shared_ptr
  channel_->enableWriting();

  connectTimer_->set(connectTimeoutMs_/1000.0, 0.);
  connectTimer_->start();
}

int Connector::removeAndResetChannel()
{
  LOG_TRACE << "Connector::removeAndResetChannel at " << this << " " << serverAddr_.toString();

  channel_->disableAll();
  channel_->remove();
  int sockfd = channel_->fd();
  // Can't reset channel_ here, because we are inside Channel::handleEvent
  loop_->queueInLoop(boost::bind(&Connector::resetChannel, shared_from_this())); // FIXME: unsafe
  return sockfd;
}

void Connector::resetChannel()
{
  LOG_TRACE << "Connector::resetChannel " << this << " " << serverAddr_.toString();
  channel_.reset();
}

void Connector::handleWrite()
{
  LOG_TRACE << "Connector::handleWrite " << getState() << " at " << this << " " << serverAddr_.toString();

  if (getState() == kConnecting)
  {
    connectTimer_->stop();

    int sockfd = removeAndResetChannel();
    int err = sockets::getSocketError(sockfd);
    if (err)
    {
      LOG_WARN << "Connector::handleWrite - SO_ERROR = "
               << err << " " << strerror_tl(err) << " " << serverAddr_.toString();
      if (!newSockConnectionErrorCallback_(err))
      {
        connect_ = false;
      }
      retry(sockfd);
    }
    else if (sockets::isSelfConnect(sockfd))
    {
      LOG_WARN << "Connector::handleWrite - Self connect " << serverAddr_.toString();
      if (!newSockConnectionErrorCallback_(ECONNSELF))
      {
        connect_ = false;
      }
      retry(sockfd);
    }
    else
    {
      setState(kConnected);
      if (connect_)
      {
        newSockConnectionCallback_(sockfd);
        lastConnectedTimestamp_ = Timestamp::now();
      }
      else
      {
        sockets::close(sockfd);
      }
    }
  }
  else
  {
    // what happened?
    assert(getState() == kDisconnected);
  }
}

void Connector::handleError()
{
  LOG_ERROR << "Connector::handleError state=" << getState() << " at " << this << " " << serverAddr_.toString();
  if (getState() == kConnecting)
  {
    connectTimer_->stop();

    int sockfd = removeAndResetChannel();
    int err = sockets::getSocketError(sockfd);
    LOG_TRACE << "SO_ERROR = " << err << " " << strerror_tl(err) << " " << serverAddr_.toString();

    if (!newSockConnectionErrorCallback_(err))
    {
      connect_ = false;
    }

    retry(sockfd);
  }
}

void Connector::handleConnectTimeout()
{
  LOG_ERROR << "Connector::handleConnectTimeout state=" << getState() << " at " << this << " " << serverAddr_.toString();
  if (getState() == kConnecting)
  {
    connectTimer_->stop();

    int sockfd = removeAndResetChannel();
    int err = ETIMEDOUT;
    LOG_TRACE << "SO_ERROR = " << err << " " << strerror_tl(err) << " " << serverAddr_.toString();

    if (!newSockConnectionErrorCallback_(err))
    {
      connect_ = false;
    }

    retry(sockfd);
  }
}

void Connector::retry(int sockfd)
{
  sockets::close(sockfd);
  setState(kDisconnected);
  if (connect_)
  {
    LOG_TRACE << "Connector::retry - Retry connecting to " << serverAddr_.toString()
             << " in " << retryDelayMs_ << " milliseconds. ";
    if(!retrying_)
    {
      retryTimer_->set(retryDelayMs_/1000.0, 0.);
      retryTimer_->start();
      retrying_ = true;
    }
    retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
  }
  else
  {
    LOG_TRACE << "do not connect" << " " << serverAddr_.toString();
  }
}

