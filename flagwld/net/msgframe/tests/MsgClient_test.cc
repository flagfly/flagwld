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


#include <flagwld/net/msgframe/UrlParser.h>
#include <flagwld/net/msgframe/TextMsgCodec.h>
#include <flagwld/net/msgframe/MsgRequest.h>
#include <flagwld/net/msgframe/MsgResponse.h>
#include <flagwld/net/msgframe/MsgBody.h>

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

class MsgClient : boost::noncopyable
{
 public:
  MsgClient(const EventLoopPtr& loop,
             const InetAddress& listenAddr,
             const string& nm)
    :client_(loop, listenAddr, nm)
  {
    LOG_DEBUG << "MsgClient::ctor[" << nm
           << "] at " << this <<" - client " << &client_;

    client_.setConnectionCallback(
        boost::bind(&MsgClient::onConnection, this, _1));
  }

  ~MsgClient()
  {
    LOG_DEBUG << "MsgClient::dtor[" << client_.name()
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

  void fill(const MsgRequestPtr& req)
  {
    req_ = req;
  }

 private:
  void onConnection(const SockConnectionPtr& conn)
  {
    if (conn->connected())
    {
      LOG_TRACE << "MsgClient[" << client_.name()
        << "] connected to " << client_.hostport();

      boost::shared_ptr<TextMsgResponseCodec> codec(new TextMsgResponseCodec());
      codec->setMsgResponseCallback(boost::bind(&MsgClient::onResponse, this, _1, _2)); 
      conn->setMessageCallback(
          boost::bind(&TextMsgResponseCodec::onMessage, codec, _1, _2));
      conn->setWriteCompleteCallback(
        boost::bind(&MsgClient::onWriteComplete, this, _1));

      conn->setContext(codec);

      conn->setTcpNoDelay(true);

      conn_ = conn;

      LOG_WARN << "MsgClient[" << client_.name()
        << "] starts sending to " << client_.hostport();
      req_->send(conn_);
    }
    else
    {
      LOG_TRACE << "MsgClient[" << client_.name()
        << "] disconnected from  " << client_.hostport();

      conn_.reset();
    }
  }

  void onWriteComplete(const SockConnectionPtr& conn)
  {
    LOG_INFO;
  }

  void onResponse(const SockConnectionPtr& conn, const MsgResponsePtr&resp)
  {
    Buffer buf;
    resp->appendToBuffer(&buf);
    std::cout << "[" << buf.peek() << resp->getBody()->asString() << "]" << std::endl;

    conn->forceClose();
    gloop->quit();
  }

  MsgRequestPtr req_;
  TcpClient client_;
  SockConnectionPtr conn_;
};

int main(int argc, char* argv[])
{
  uint16_t port = 80;
  const char* url = "http://127.0.0.1/hello";

  UrlParser e;

  if (argc > 1)
  {
    url = argv[1];
  }

  if (!e.Execute(url, strlen(url))) {
    std::cerr << "Error: " << url << std::endl;
    return -1;
  }

  if (e.port()) {
    port = e.port();
  }
  
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

  //MsgClient client(loop, InetAddress(e.host(), port), "dummy");
  MsgClient client(loop, InetAddress( *(static_cast<struct sockaddr_in*>(implicit_cast<void*>(res->ai_addr))) ), "dummy");
#else
  InetAddress addr(port);
  if (!InetAddress::resolve(e.host(), &addr))
  {
    return -1;
  }
  std::cout << "XXXXXXXXXXXXXX " << addr.toIpPort() << std::endl;
  MsgClient client(loop, addr, "dummy");
#endif 
  MsgRequestPtr req(new MsgRequest);
  
  std::cout << e.proto() << std::endl;
  string x = e.proto();
  boost::to_upper(x);
  req->setProto(x);
  std::cout << e.proto() << std::endl;
  req->setMajor(1);
  req->setMinor(1);
  req->setMethod( "GET" );
  req->setPath( e.path() );
  req->setQuery( e.query() );
  req->addHeader( "Host", e.host() );
  req->setCloseConnection(false);
  
  Buffer buf;
  req->appendToBuffer(&buf);
  std::cout << "X[" << buf.peek() << "]X" << std::endl;
  
  client.fill( req );
  client.start();

  loop->loop();

  client.stop();
}

