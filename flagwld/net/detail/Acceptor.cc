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


#include <flagwld/net/detail/Acceptor.h>

#include <flagwld/base/Logging.h>
#include <flagwld/net/EventLoop.h>
#include <flagwld/net/SockAddress.h>
#include <flagwld/net/SocketsOps.h>

#include <boost/bind.hpp>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

using namespace flagwld;
using namespace flagwld::net;

Acceptor::Acceptor(const EventLoopPtr& loop, const SockAddress& listenAddr, bool reuseport)
  : loop_(loop),
    acceptSocket_(sockets::createStreamNonblockingOrDie(listenAddr.family())),
    acceptChannel_(loop, acceptSocket_.fd()),
    listenAddr_(listenAddr),
    listenning_(false),
    idleFd_(::open("/dev/null", O_RDONLY ))
{
  assert(idleFd_ >= 0);

  if (listenAddr_.family() != AF_UNIX)
  {
    sockets::setReuseAddr(acceptSocket_.fd(), true);
    sockets::setReusePort(acceptSocket_.fd(), reuseport);
  }

  acceptChannel_.setReadCallback(
      boost::bind(&Acceptor::handleRead, this));
  {
    int val=fcntl(idleFd_,F_GETFD);
    val|=FD_CLOEXEC;
    fcntl(idleFd_,F_SETFD,val);
  }
}

Acceptor::~Acceptor()
{
  LOG_TRACE << "Acceptor::~Acceptor [" << this << "] stopping at: " << listenAddr_.toString();;

  acceptChannel_.disableAll();
  acceptChannel_.remove();
  ::close(idleFd_);

  if (listenAddr_.family() == AF_UNIX)
  {
    ::unlink(listenAddr_.toString().c_str());
  }
}

void Acceptor::listen()
{
  LOG_TRACE << "Acceptor::listen [" << this << "] listening at: " << listenAddr_.toString();

  loop_->assertInLoopThread();
  listenning_ = true;
  acceptSocket_.bindAddress(listenAddr_);
  acceptSocket_.listen();
  acceptChannel_.enableReading();
}

void Acceptor::handleRead()
{
  loop_->assertInLoopThread();
  SockAddress peerAddr;
  int connfd = -1;
  while(true) 
  {
    connfd = acceptSocket_.accept(&peerAddr);
    if (connfd >= 0)
    {
      if (newConnectionCallback_)
      {
        newConnectionCallback_(connfd, peerAddr);
      }
      else
      {
        sockets::close(connfd);
      }
    }
    else
    {
      // Read the section named "The special problem of
      // accept()ing when you can't" in libev's doc.
      // By Marc Lehmann, author of libev.
      if (errno == EMFILE)
      {
        LOG_SYSERR << "at Acceptor[" << listenAddr_.toString() << "]";

        ::close(idleFd_);
        idleFd_ = ::accept(acceptSocket_.fd(), NULL, NULL);
        ::close(idleFd_);
        idleFd_ = ::open("/dev/null", O_RDONLY );
        {
          int val=fcntl(idleFd_,F_GETFD);
          val|=FD_CLOEXEC;
          fcntl(idleFd_,F_SETFD,val);
        }
      }
      break;
    }
  }
}

