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


#include <flagwld/net/SockConnection.h>

#include <flagwld/base/Logging.h>
#include <flagwld/base/WeakCallback.h>
#include <flagwld/net/Channel.h>
#include <flagwld/net/EventLoop.h>
#include <flagwld/net/SocketsOps.h>
#include <flagwld/net/detail/Socket.h>

#include <boost/bind.hpp>

#include <errno.h>
#include <stdio.h> //snprintf

using namespace flagwld;
using namespace flagwld::net;

void flagwld::net::defaultConnectionIdleCallback(const SockConnectionPtr& conn)
{
  LOG_WARN << conn->localAddress().toString() << " -> "
            << conn->peerAddress().toString() << " is "
            << "Idle: " << conn->idletime();
  conn->forceClose();
}

void flagwld::net::defaultConnectionCallback(const SockConnectionPtr& conn)
{
  LOG_TRACE << conn->localAddress().toString() << " -> "
            << conn->peerAddress().toString() << " is "
            << (conn->connected() ? "UP" : "DOWN");
  // do not call conn->forceClose(), because some users want to register message callback only.
}

bool flagwld::net::defaultConnectErrorCallback(const SockAddress& serverAddr)
{
  bool retry = true;
  LOG_TRACE << "Connect to " << serverAddr.toString() << " Error.";
  return retry;
}

void flagwld::net::defaultMessageCallback(const SockConnectionPtr& conn, Buffer* buf)
{
  LOG_TRACE << conn->localAddress().toString() << " -> "
            << conn->peerAddress().toString() << " recv "
            << buf->readableBytes();
  buf->retrieveAll();
}

void flagwld::net::defaultWriteCompleteCallback(const SockConnectionPtr& conn)
{
  LOG_TRACE << conn->localAddress().toString() << " -> "
            << conn->peerAddress().toString() << " write complete";
}

void flagwld::net::defaultHighWaterMarkCallback(const SockConnectionPtr& conn, size_t high)
{
  LOG_TRACE << conn->localAddress().toString() << " -> "
            << conn->peerAddress().toString() << " write high wather: " << high;
}

SockConnection::SockConnection(const EventLoopPtr& loop,
                             const string& nameArg,
                             int sockfd,
                             const SockAddress& localAddr,
                             const SockAddress& peerAddr)
  : loop_(CHECK_NOTNULL(get_pointer(loop))?loop:loop),
    name_(nameArg),
    state_(kConnecting),
    reading_(true),
    socket_(new Socket(sockfd)),
    channel_(new Channel(loop, sockfd)),
    localAddr_(localAddr),
    peerAddr_(peerAddr),
    highWaterMark_(64*1024*1024),
    prevHighWaterMark_(false),
    idleTimeout_(0.0),
    idleTiming_(false),
    bulksize_(0)
{
  LOG_DEBUG << "SockConnection::ctor[" <<  name_ << "] at " << this
            << " fd=" << sockfd << " - channel at " << get_pointer(channel_);

  channel_->setReadCallback(
      boost::bind(&SockConnection::handleRead, this));
  channel_->setWriteCallback(
      boost::bind(&SockConnection::handleWrite, this));
  channel_->setCloseCallback(
      boost::bind(&SockConnection::handleClose, this));
  channel_->setErrorCallback(
      boost::bind(&SockConnection::handleError, this));

  sockets::setKeepAlive(socket_->fd(), true);

}

SockConnection::~SockConnection()
{
  LOG_DEBUG << "SockConnection::dtor[" <<  name_ << "] at " << this
            << " fd=" << channel_->fd() 
            << " - channel at " << get_pointer(channel_)
            << " state=" << stateToString();
  assert(state_.get() == kDisconnected);
}

string SockConnection::getInfoString() const
{
  char buf[1024];
  buf[0] = '\0';

  if (localAddr_.family() != AF_UNIX)
  {
    struct tcp_info tcpi;
    bool ok = sockets::getTcpInfo(socket_->fd(), &tcpi);
    if (ok)
    {
      snprintf(buf, sizeof(buf)-1, "unrecovered=%u "
             "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
             "lost=%u retrans=%u rtt=%u rttvar=%u "
             "sshthresh=%u cwnd=%u total_retrans=%u",
             tcpi.tcpi_retransmits,  // Number of unrecovered [RTO] timeouts
             tcpi.tcpi_rto,          // Retransmit timeout in usec
             tcpi.tcpi_ato,          // Predicted tick of soft clock in usec
             tcpi.tcpi_snd_mss,
             tcpi.tcpi_rcv_mss,
             tcpi.tcpi_lost,         // Lost packets
             tcpi.tcpi_retrans,      // Retransmitted packets out
             tcpi.tcpi_rtt,          // Smoothed round trip time in usec
             tcpi.tcpi_rttvar,       // Medium deviation
             tcpi.tcpi_snd_ssthresh,
             tcpi.tcpi_snd_cwnd,
             0u/*tcpi.tcpi_total_retrans*/);  // Total retransmits for entire connection
    }
  }

  return buf;
}

void SockConnection::enableIdleTimer( double tmo )
{
  if (loop_->isInLoopThread())
  {
    enableIdleTimerInLoop(tmo);
  }
  else
  {
    loop_->runInLoop(
      boost::bind(&SockConnection::enableIdleTimerInLoop,
                      shared_from_this(),
                      tmo));
  }
}

void SockConnection::enableIdleTimerInLoop( double tmo )
{
  assert( idleCallback_ );
  loop_->assertInLoopThread();

  idleTimeout_ = tmo;

  if( !idleTiming_ )
  {
    idletimer_.reset(new Timer(loop_, idleTimeout_, idleTimeout_));
    idletimer_->setTimerCallback(
      boost::bind(&SockConnection::handleIdle, this));

    idletimer_->tie(shared_from_this());
    idletimer_->start();
    idleTiming_ = true;
  }
  else
  {
    idletimer_->set(idleTimeout_, idleTimeout_);
    idletimer_->restart();
  }
}

void SockConnection::disableIdleTimer()
{
  if (loop_->isInLoopThread())
  {
    disableIdleTimerInLoop();
  }
  else
  {
    loop_->runInLoop(
      boost::bind(&SockConnection::disableIdleTimerInLoop,
                      shared_from_this()
                      ));
  }
}

void SockConnection::disableIdleTimerInLoop()
{
  loop_->assertInLoopThread();

  if( idleTiming_ )
  {
    idletimer_->stop(); 
    idletimer_.reset();
    idleTiming_ = false;
    idleTimeout_ = 0.;
  }
}

void SockConnection::send(const void* data, int len)
{
  send(StringPiece(static_cast<const char*>(data), len));
}

void SockConnection::send(const StringPiece& message)
{
  if (getState() == kConnected)
  {
    if (loop_->isInLoopThread())
    {
      sendInLoop(message);
    }
    else
    {
      loop_->runInLoop(
          boost::bind(&SockConnection::sendInLoop,
                      shared_from_this(),
                      message.as_string()));
                    //std::forward<string>(message)));
    }
  }
}

// FIXME efficiency!!!
void SockConnection::send(Buffer* buf)
{
  if (getState() == kConnected)
  {
    if (loop_->isInLoopThread())
    {
      sendInLoop(buf->peek(), buf->readableBytes());
      buf->retrieveAll();
    }
    else
    {
      loop_->runInLoop(
          boost::bind(&SockConnection::sendInLoop,
                      shared_from_this(),
                      buf->retrieveAllAsString()));
                    //std::forward<string>(message)));
    }
  }
}

void SockConnection::sendInLoop(const StringPiece& message)
{
  sendInLoop(message.data(), message.size());
}

void SockConnection::sendInLoop(const void* data, size_t len)
{
  loop_->assertInLoopThread();
  ssize_t nwrote = 0;
  size_t remaining = len;
  size_t cansend = len;
  bool faultError = false;
  if (getState() == kDisconnected)
  {
    LOG_WARN << "disconnected, give up writing";
    return;
  }
  // if no thing in output queue, try writing directly
  if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
  {
    if (bulksize_>0)
    {
      cansend = cansend<=bulksize_?cansend:bulksize_;
    }
    nwrote = sockets::write(channel_->fd(), data, cansend);
    if (nwrote >= 0)
    {
      remaining = len - nwrote;
      if (remaining == 0 && writeCompleteCallback_ && prevHighWaterMark_)
      {
        prevHighWaterMark_ = false;
        loop_->queueInLoop(boost::bind(writeCompleteCallback_, shared_from_this()));
      }
      if(nwrote > 0 && idleTiming_)
      {
        idletimer_->restart();
      }
    }
    else // nwrote < 0
    {
      nwrote = 0;
      if (errno != EWOULDBLOCK)
      {
        LOG_SYSERR << "SockConnection::sendInLoop " << peerAddress().toString();
        if (errno != EINTR || errno == ECONNRESET) // FIXME: any others?
        {
          faultError = true;
        }
      }
    }
  }

  assert(remaining <= len);
  if (!faultError && remaining > 0)
  {
    LOG_TRACE << "I am going to write more data";
    size_t oldLen = outputBuffer_.readableBytes();
    if (oldLen + remaining >= highWaterMark_
        && oldLen < highWaterMark_
        && highWaterMarkCallback_)
    {
      prevHighWaterMark_ = true;
      loop_->runInLoop(boost::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
    }
    outputBuffer_.append(static_cast<const char*>(data)+nwrote, remaining);
    if (!channel_->isWriting())
    {
      channel_->enableWriting();
    }
  }
}

void SockConnection::shutdown()
{
  if ( compareAndSetState( kConnected, kDisconnecting) )
  {
    loop_->runInLoop(boost::bind(&SockConnection::shutdownInLoop, shared_from_this()));
  }
}

void SockConnection::shutdownInLoop()
{
  loop_->assertInLoopThread();
  if (!channel_->isWriting())
  {
    // we are not writing
    socket_->shutdownWrite();
  }
}

//void SockConnection::shutdownAndForceCloseAfter(double seconds)
//{
//  // FIXME: use compare and swap
//  if (state_ == kConnected)
//  {
//    setState(kDisconnecting);
//    loop_->runInLoop(boost::bind(&SockConnection::shutdownAndForceCloseInLoop, this, seconds));
//  }
//}

//void SockConnection::shutdownAndForceCloseInLoop(double seconds)
//{
//  loop_->assertInLoopThread();
//  if (!channel_->isWriting())
//  {
//    // we are not writing
//    socket_->shutdownWrite();
//  }
//  loop_->runAfter(
//      seconds,
//      makeWeakCallback(shared_from_this(),
//                       &SockConnection::forceCloseInLoop));
//}

void SockConnection::forceClose()
{ 
  if ( compareAndSetState( kConnected, kDisconnecting) ||
            (getState() == kDisconnecting) )
  {
    //loop_->queueInLoop(boost::bind(&SockConnection::forceCloseInLoop, shared_from_this()));
    loop_->runInLoop(boost::bind(&SockConnection::forceCloseInLoop, shared_from_this()));
  } 
} 

//void SockConnection::forceCloseWithDelay(double seconds)
//{
//  if (state_ == kConnected || state_ == kDisconnecting)
//  {
//    setState(kDisconnecting);
//    loop_->runAfter(
//        seconds,
//        makeWeakCallback(shared_from_this(),
//                         &SockConnection::forceClose));  // not forceCloseInLoop to avoid race condition
//  }
//}
  
void SockConnection::forceCloseInLoop()
{   
  loop_->assertInLoopThread();
  if ( getState() == kConnected || getState() == kDisconnecting)
  {
    // as if we received 0 byte in handleRead();
    handleClose();
  }
}

const char* SockConnection::stateToString()
{
  StateE e = getState();
  switch (e)
  {
    case kDisconnected:
      return "kDisconnected";
   case kConnecting:
      return "kConnecting";
    case kConnected:
      return "kConnected";
    case kDisconnecting:
      return "kDisconnecting";
   default:
      return "unknown state";
  }
}

void SockConnection::setTcpNoDelay(bool on)
{
  if (localAddr_.family() != AF_UNIX)
  {
    sockets::setTcpNoDelay(socket_->fd(), on);
  }
}

void SockConnection::setSndBuffer(int buf_len)
{
  sockets::setSndBuffer(socket_->fd(), buf_len);
}

void SockConnection::setRcvBuffer(int buf_len)
{
  sockets::setRcvBuffer(socket_->fd(), buf_len);
}

void SockConnection::startRead()
{
  loop_->runInLoop(boost::bind(&SockConnection::startReadInLoop, shared_from_this()));
}

void SockConnection::startReadInLoop()
{
  loop_->assertInLoopThread();
  if (!reading_ || !channel_->isReading())
  {
    channel_->enableReading();
    reading_ = true;
  }
}

void SockConnection::stopRead()
{
  loop_->runInLoop(boost::bind(&SockConnection::stopReadInLoop, shared_from_this()));
}

void SockConnection::stopReadInLoop()
{
  loop_->assertInLoopThread();
  if (reading_ || channel_->isReading())
  {
    channel_->disableReading();
    reading_ = false;
  }
}

void SockConnection::connectEstablished()
{
  LOG_TRACE << "SockConnection::connectEstablished[" << name_ << " at "<< this;
  loop_->assertInLoopThread();
  assert(getState() == kConnecting);
  setState(kConnected);
  channel_->tie(shared_from_this());
  channel_->enableReading();
  if( idleTiming_ )
  {
    idletimer_->restart();
  }

  connectionCallback_(shared_from_this());
}

void SockConnection::connectDestroyed()
{
  LOG_TRACE << "SockConnection::connectDestroyed[" << name_ << " at "<< this;
  loop_->assertInLoopThread();
  if ( compareAndSetState( kConnected, kDisconnected) )
  {
    channel_->disableAll();

    connectionCallback_(shared_from_this());
  }

  setContext(boost::any());

  channel_->remove();
  if(idleTiming_)
  {
    idletimer_->stop();
    idleTiming_=false;
  }
}

void SockConnection::handleRead()
{
  loop_->assertInLoopThread();
  int savedErrno = 0;
  ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
  if (n > 0)
  {
    messageCallback_(shared_from_this(), &inputBuffer_);
    if(idleTiming_)
    {
      idletimer_->restart();
    }
  }
  else if (n == 0)
  {
    // none bytes received
    messageCallback_(shared_from_this(), &inputBuffer_);
    handleClose();
  }
  else
  {
    LOG_SYSERR << "SockConnection::handleRead - " << peerAddress().toString();
    switch (savedErrno)
    {
      case EINTR:
      case EAGAIN:
        break;
      default:
        errno = savedErrno;
        handleError();
        break;
    }
  }
}

void SockConnection::handleWrite()
{
  loop_->assertInLoopThread();
  if (channel_->isWriting())
  {
    size_t cansend = outputBuffer_.readableBytes();
    if (bulksize_>0)
    {
      cansend = cansend<=bulksize_?cansend:bulksize_;
    }
    ssize_t n = sockets::write(channel_->fd(),
                               outputBuffer_.peek(),
                               cansend);
    if (n > 0)
    {
      outputBuffer_.retrieve(n);
      if (outputBuffer_.readableBytes() == 0)
      {
        channel_->disableWriting();
        if (writeCompleteCallback_ && prevHighWaterMark_)
        {
          prevHighWaterMark_ = false;
          loop_->queueInLoop(boost::bind(writeCompleteCallback_, shared_from_this()));
        }
        if (getState() == kDisconnecting)
        {
          shutdownInLoop();
        }
      }
      else
      {
        LOG_TRACE << "I am going to write more data";
      }
      if(idleTiming_)
      {
        idletimer_->restart();
      }
    }
    else
    {
      LOG_SYSERR << "SockConnection::handleWrite - " << peerAddress().toString();
      // if (getState() == kDisconnecting)
      // {
      //   shutdownInLoop();
      // }
    }
  }
  else
  {
    LOG_TRACE << "SockConnection fd = " << channel_->fd()
              << " is down, no more writing";
  }
}

void SockConnection::handleClose()
{
  LOG_TRACE << "SockConnection::handleClose[" << name_ << " at "<< this << "fd = " << channel_->fd() << " state = " << stateToString();

  loop_->assertInLoopThread();
  assert(getState() == kConnected || getState() == kDisconnecting);
  // we don't close fd, leave it to dtor, so we can find leaks easily.
  setState(kDisconnected);
  channel_->disableAll();
  if(idleTiming_)
  {
    idletimer_->stop();
    idleTiming_=false;
  }
  SockConnectionPtr guardThis(shared_from_this());
  connectionCallback_(guardThis);

  // must be the last line
  closeCallback_(guardThis);
}

void SockConnection::handleError()
{
  int err = sockets::getSocketError(channel_->fd());
  LOG_TRACE << "SockConnection::handleError [" << name_
            << "] - SO_ERROR = " << err << " " << strerror_tl(err);
  handleClose();
}

void SockConnection::handleIdle()
{
  loop_->assertInLoopThread();
  LOG_TRACE << "fd = " << channel_->fd() << " state = " << getState();
  assert(getState() == kConnected || getState() == kDisconnecting);

  SockConnectionPtr guardThis(shared_from_this());
  idleCallback_(guardThis);
}
