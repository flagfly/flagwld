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


#include <flagwld/net/msgframe/TextLineCodec.h>

#include <flagwld/net/TcpClient.h>
#include <flagwld/net/EventLoop.h>
#include <flagwld/base/Logging.h>

#include <boost/noncopyable.hpp>
#include <boost/make_shared.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <iostream>
#include <map>

using namespace flagwld;
using namespace flagwld::net;

EventLoopPtr gloop;

uint32_t kIdleConnect = 120;

struct LineCtx
{
  boost::shared_ptr<TextLineCodec> codec;
  bool handshaked;
};
typedef boost::shared_ptr<LineCtx> LineCtxPtr;

class IrcClient : boost::noncopyable
{
 public:
  IrcClient(const EventLoopPtr& loop,
             const InetAddress& listenAddr,
             const string& nm)
    :client_(loop, listenAddr, nm)
  {
    LOG_DEBUG << "IrcClient::ctor[" << nm
           << "] at " << this <<" - client " << &client_;

    client_.setConnectionCallback(
        boost::bind(&IrcClient::onConnection, this, _1));
  }

  ~IrcClient()
  {
    LOG_DEBUG << "IrcClient::dtor[" << client_.name()
           << "] at " << this <<" - client " << &client_;
  }

  void start()
  {
    client_.connect();
  }

  void stop()
  {
    client_.stop();
  }

 private:
  void onConnection(const SockConnectionPtr& conn)
  {
    if (conn->connected())
    {
      LOG_TRACE << "IrcClient[" << client_.name()
        << "] connected to " << client_.hostport();

      LineCtxPtr ctx(new LineCtx);
      if (!ctx)
      {
        conn->shutdown();
        return ;
      }
      ctx->codec.reset(new TextLineCodec);
      if (!ctx->codec)
      {
        conn->shutdown();
        return ;
      }
      ctx->handshaked = false;
 
      ctx->codec->setLineMsgCallback(boost::bind(&IrcClient::onLineMessage, this, _1, _2, _3));
      conn->setMessageCallback(
          boost::bind(&TextLineCodec::onMessage, ctx->codec, _1, _2));
 
      conn->setContext(ctx);
      conn->enableIdleTimer(kIdleConnect*10);
    }
    else
    {
      LOG_TRACE << "IrcClient[" << client_.name()
        << "] disconnected from  " << client_.hostport();

      conn_.reset();
    }
  }

  void onLineMessage(const SockConnectionPtr& conn, const char* data, size_t len)
  {
    LineCtxPtr ctx =
       *(boost::any_cast<LineCtxPtr>(conn->getMutableContext()));

     if (!ctx->handshaked)
     {
       conn->send("OK\r\n");
       ctx->handshaked = true;

       LOG_INFO << "[" << string(data, len) << "]" << len << " handshake ok.";
     }
     else
     {
       LOG_INFO << "[" << string(data, len) << "]" << len << " normal message.";
     }
  }

  void onWriteComplete(const SockConnectionPtr& conn)
  {
    LOG_INFO;
  }

  TcpClient client_;
  SockConnectionPtr conn_;
};

int main(int argc, char* argv[])
{
  uint16_t port = 80;
  const char* ip = "127.0.0.1";

  EventLoopPtr loop = boost::make_shared<EventLoop>();
  gloop = loop;
#if 0
  struct addrinfo hints;
  struct addrinfo* res;
                        
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  char port_str[16];
  sprintf(port_str, "%hu", port);
  int ret = getaddrinfo(e.host().c_str(), port_str, &hints, &res);
  if (ret != 0) {
    return -1;
  }

  //IrcClient client(loop, InetAddress(e.host(), port), "dummy");
  IrcClient client(loop, InetAddress( *(static_cast<struct sockaddr_in*>(implicit_cast<void*>(res->ai_addr))) ), "dummy");
#else
  InetAddress addr(port);
  if (!InetAddress::resolve(ip, &addr))
  {
    return -1;
  }
  std::cout << "XXXXXXXXXXXXXX " << addr.toIpPort() << std::endl;
  IrcClient client(loop, addr, "dummy");
#endif 
  
  client.start();

  loop->loop();

  client.stop();
}

