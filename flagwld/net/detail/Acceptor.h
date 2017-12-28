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


#ifndef FLAGWLD_NET_ACCEPTOR_H
#define FLAGWLD_NET_ACCEPTOR_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <flagwld/net/Channel.h>
#include <flagwld/net/SockAddress.h>
#include <flagwld/net/detail/Socket.h>

namespace flagwld
{
namespace net
{

class EventLoop;
typedef boost::shared_ptr<EventLoop> EventLoopPtr;
class SockAddress;

///
/// Acceptor of incoming  connections.
///
class Acceptor : boost::noncopyable
{
 public:
  typedef boost::function<void (int sockfd,
                                const SockAddress&)> NewConnectionCallback;

  Acceptor(const EventLoopPtr& loop, const SockAddress& listenAddr, bool reuseport);
  ~Acceptor();

  void setNewConnectionCallback(const NewConnectionCallback& cb)
  { newConnectionCallback_ = cb; }

  bool listenning() const { return listenning_; }
  void listen();

 private:
  void handleRead();

  EventLoopPtr loop_;
  Socket acceptSocket_;
  Channel acceptChannel_;
  NewConnectionCallback newConnectionCallback_;
  SockAddress listenAddr_;
  bool listenning_;
  int idleFd_;
};

}
}

#endif  // FLAGWLD_NET_ACCEPTOR_H
