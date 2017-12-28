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


#include <flagwld/net/TcpServer.h>

#include <flagwld/base/Logging.h>
#include <flagwld/base/Thread.h>
#include <flagwld/net/EventLoop.h>
#include <flagwld/net/SockAddress.h>
#include <flagwld/net/msgframe/TextLineCodec.h>

#include <boost/bind.hpp>

#include <utility>

#include <stdio.h>
#include <unistd.h>

using namespace flagwld;
using namespace flagwld::net;

int16_t bindPort = 2000;

uint32_t kIdleConnect = 120;

struct LineCtx
{
  boost::shared_ptr<TextLineCodec> codec;
  bool handshaked;
};
typedef boost::shared_ptr<LineCtx> LineCtxPtr;

class LineServer
{
 public:
  LineServer(const EventLoopPtr& loop, const SockAddress& listenAddr)
    : loop_(loop),
      server_(loop, listenAddr, "LineServer")
  {
    server_.setConnectionCallback(
        boost::bind(&LineServer::onConnection, this, _1));
  }

  void start()
  {
    server_.start();
  }
  void stop()
  {
    server_.stop();
    loop_->quit();
  }

 private:
  void onConnection(const SockConnectionPtr& conn)
  {
    LOG_TRACE << conn->peerAddress().toString() << " -> "
        << conn->localAddress().toString() << " is "
        << (conn->connected() ? "UP" : "DOWN");
    //LOG_INFO << conn->getInfoString();

    if (conn->connected())
    {
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
  
      ctx->codec->setLineMsgCallback(boost::bind(&LineServer::onLineMessage, this, _1, _2, _3));
      conn->setMessageCallback(
          boost::bind(&TextLineCodec::onMessage, ctx->codec, _1, _2));
  
      conn->setContext(ctx);
      conn->enableIdleTimer(kIdleConnect*10);
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

  EventLoopPtr loop_;
  SockServer server_;
};

static void signal_handler(int sig, sighandler_t handler)
{
  struct sigaction action;
  action.sa_handler = handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  sigaction(sig, &action, NULL);
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
static void ignore_signals()
{
  signal_handler(SIGHUP,  SIG_IGN);
  signal_handler(SIGTERM, SIG_IGN);
  signal_handler(SIGPIPE, SIG_IGN);
  //signal_handler(SIGXFSZ, SIG_IGN);
}
#pragma GCC diagnostic error "-Wold-style-cast"

int main(int argc, char* argv[])
{
  LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
  LOG_INFO << "sizeof TcpConnection = " << sizeof(SockConnection);

  ignore_signals();

  if (argc > 1)
  {
    bindPort = static_cast<int16_t>(atoi(argv[1]));
  }
  EventLoopPtr loop( new EventLoop);
  SockAddress listenAddr(bindPort);
  LineServer server(loop, listenAddr);

  server.start();

  loop->loop();
}

