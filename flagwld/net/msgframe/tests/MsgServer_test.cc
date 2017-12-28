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


#include <flagwld/net/msgframe/TextMsgCodec.h>
#include <flagwld/net/msgframe/MsgRequest.h>
#include <flagwld/net/msgframe/MsgResponse.h>
#include <flagwld/net/msgframe/MsgBody.h>
#include <flagwld/net/TcpServer.h>
#include <flagwld/net/Signal.h>

#include <flagwld/net/EventLoop.h>
#include <flagwld/base/Logging.h>

#include <boost/make_shared.hpp>
#include <boost/bind.hpp>

#include <iostream>
#include <map>
#include <stdio.h>

using namespace flagwld;
using namespace flagwld::net;

extern char favicon[555];
bool benchmark = true;

class MsgServer : boost::noncopyable
{
 public:
  MsgServer(const EventLoopPtr& loop,
             const InetAddress& listenAddr,
             const string& name,
             TcpServer::Option option = TcpServer::kNoReusePort):
             server_(loop, listenAddr, name, option)
  {
    server_.setConnectionCallback(
        boost::bind(&MsgServer::onConnection, this, _1));
  }

  ~MsgServer()
  {
  }

  EventLoopPtr getLoop() const { return server_.getLoop(); }

  void setThreadNum(int numThreads)
  {
    server_.setThreadNum(numThreads);
  }

  void start()
  {
    LOG_WARN << "MsgServer[" << server_.name()
      << "] starts listenning on " << server_.hostport();
    server_.start();
  }

 private:
  TcpServer server_;

  void onConnection(const SockConnectionPtr& conn)
  {
    if (conn->connected())
    {
      boost::shared_ptr<TextMsgRequestCodec> codec(new TextMsgRequestCodec());
      codec->setMsgRequestCallback(boost::bind(&MsgServer::onRequest, this, _1, _2, _3));
      conn->setContext(codec);
      conn->setMessageCallback(
          boost::bind(&TextMsgRequestCodec::onMessage, codec, _1, _2));
      conn->setTcpNoDelay(true);
    }
  }
  void onRequest(const SockConnectionPtr& conn, const MsgRequestPtr& req, const MsgResponsePtr& resp)
  {
    resp->setProto(req->proto());
    resp->setMajor(req->major());
    resp->setMinor(req->minor());
#if 0
    Buffer buf;
    req->appendToBuffer(&buf);
    std::cout << "[" << buf.peek() << "]" << std::endl;
    //std::cout << "Proto: " << req->proto() << " Ver: " << req->major() << ":" << req->minor() << " Method: " << req->method() << " Path: " << req->path() << std::endl;

    if (!benchmark)
    {
      const boost::unordered_map<string, string>& headers = req->headers();
      for (boost::unordered_map<string, string>::const_iterator it = headers.begin();
           it != headers.end();
           ++it)
      {
        std::cout << it->first << ": " << it->second << std::endl;
      }
    }
#endif
    if (req->path() == "/")
    {
      if (req->nbody()>0)
      {
         std::cout << req->proto() << "/" << req->major() << "." << req->minor() << " " << req->method() << " " << req->uri() << " " << req->nbody() << " "
           << "[" << req->getBody()->data() << "]" << std::endl;
      }
      resp->setCode(200);
      resp->setPhrase("OK");
      resp->setContentType("text/html");
      resp->addHeader("Server", "Flagwld");
      string now = Timestamp::now().toRFC822FormattedString();
      resp->setBody(new MsgBody);
      resp->appendBody("<html><head><title>This is title</title></head>"
        "<body><h1>Hello</h1>Now is " + now +
        "</body></html>");
    }
    else if (req->path() == "/task")
    {
      resp->setCode(200);
      resp->setPhrase("OK");
      resp->setContentType("text/html");
      resp->setCloseConnection(false);
      resp->addHeader("Server", "Flagwld");
      resp->addHeader("Date", Timestamp::now().toRFC822FormattedString());

      resp->setChunkedContent(true);
      resp->setCloseConnection(false);
      resp->send(conn);

      for (int i=0; i<10; ++i)
      {
        resp->sendBody(conn, "a", 1);
      }
      resp->sendBody(conn, 0, 0);

      return ;
    }
    else if (req->path() == "/init")
    {
      resp->setCode(200);
      resp->setPhrase("OK");
      resp->setContentType("text/html");
      resp->setCloseConnection(false);
      resp->addHeader("Server", "Flagwld");
      resp->addHeader("Date", Timestamp::now().toRFC822FormattedString());
    }
    else if (req->path() == "/hello")
    {
      resp->setCode(200);
      resp->setPhrase("OK");
      resp->setContentType("text/plain");
      resp->addHeader("Server", "Flagwld");
      resp->setBody(new MsgBody);
      resp->appendBody("hello, world!");
    }
    else if (req->path() == "/proxy")
    {
      resp->setCode(301);
      resp->setPhrase("Moved Permanently");
      resp->setContentType("text/plain");
      resp->addHeader("Server", "Flagwld");
      resp->addHeader("Location", "http://10.10.82.214/bin/x.sh");
    }
    else if (req->path() == "/proxy1")
    {
      resp->setCode(302);
      resp->setPhrase("Found");
      resp->setContentType("text/plain");
      resp->addHeader("Server", "Flagwld");
      resp->addHeader("Location", "http://10.10.82.214/bin/x.sh");
    }
    else
    {
      resp->setCode(404);
      resp->setPhrase("Not Found");
      resp->setCloseConnection(true);
    }

  /*buf.retrieveAll();
    resp->appendToBuffer(&buf);
    std::cout << "[" << string(buf.peek(), buf.readableBytes()) << "]" << std::endl;
  */

    resp->setChunkedContent(false);
    resp->setCloseConnection(false);
    resp->send(conn);


  resp->setTimestamp(Timestamp::now());
  string serverv = resp->getHeader("Server");
  LOG_INFO << conn->localAddress().toString() << " "
           << conn->peerAddress().toString() << " "
           << req->proto() << " "
           << req->method() << " "
           << req->uri() << " - - \""
           << (serverv.empty()?"-":serverv) << "\" "
           << resp->code() << " "
           << Fmt("%.4f", timeDifference(resp->timestamp(), req->timestamp())) << " "
           << resp->nparsed() << " "
           << resp->nbody();


  }
};

static void signal_handler(int sig, sighandler_t handler)
{
  struct sigaction action;
  action.sa_handler = handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  sigaction(sig, &action, NULL);
}

static void ignore_signals()
{
  signal_handler(SIGHUP,  SIG_IGN);
  signal_handler(SIGTERM, SIG_IGN);
  signal_handler(SIGPIPE, SIG_IGN);
}

void Print()
{
  printf("Print(): pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
}

int main(int argc, char* argv[])
{
  //Logger::setLogLevel(Logger::TRACE);

  ignore_signals();

  int numThreads = 0;
  if (argc > 1)
  {
    benchmark = true;
    //Logger::setLogLevel(Logger::WARN);
    numThreads = atoi(argv[1]);
  }

  EventLoopPtr loop = boost::make_shared<EventLoop>();
  MsgServer server(loop, InetAddress(8000), "dummy");
  server.setThreadNum(numThreads);
  server.start();

  Signal s(loop, SIGPIPE);
  s.setSignalCallback(boost::bind(Print));
  s.start();

  loop->loop();
}

char favicon[555] = {
  '\x89', 'P', 'N', 'G', '\xD', '\xA', '\x1A', '\xA',
  '\x0', '\x0', '\x0', '\xD', 'I', 'H', 'D', 'R',
  '\x0', '\x0', '\x0', '\x10', '\x0', '\x0', '\x0', '\x10',
  '\x8', '\x6', '\x0', '\x0', '\x0', '\x1F', '\xF3', '\xFF',
  'a', '\x0', '\x0', '\x0', '\x19', 't', 'E', 'X',
  't', 'S', 'o', 'f', 't', 'w', 'a', 'r',
  'e', '\x0', 'A', 'd', 'o', 'b', 'e', '\x20',
  'I', 'm', 'a', 'g', 'e', 'R', 'e', 'a',
  'd', 'y', 'q', '\xC9', 'e', '\x3C', '\x0', '\x0',
  '\x1', '\xCD', 'I', 'D', 'A', 'T', 'x', '\xDA',
  '\x94', '\x93', '9', 'H', '\x3', 'A', '\x14', '\x86',
  '\xFF', '\x5D', 'b', '\xA7', '\x4', 'R', '\xC4', 'm',
  '\x22', '\x1E', '\xA0', 'F', '\x24', '\x8', '\x16', '\x16',
  'v', '\xA', '6', '\xBA', 'J', '\x9A', '\x80', '\x8',
  'A', '\xB4', 'q', '\x85', 'X', '\x89', 'G', '\xB0',
  'I', '\xA9', 'Q', '\x24', '\xCD', '\xA6', '\x8', '\xA4',
  'H', 'c', '\x91', 'B', '\xB', '\xAF', 'V', '\xC1',
  'F', '\xB4', '\x15', '\xCF', '\x22', 'X', '\x98', '\xB',
  'T', 'H', '\x8A', 'd', '\x93', '\x8D', '\xFB', 'F',
  'g', '\xC9', '\x1A', '\x14', '\x7D', '\xF0', 'f', 'v',
  'f', '\xDF', '\x7C', '\xEF', '\xE7', 'g', 'F', '\xA8',
  '\xD5', 'j', 'H', '\x24', '\x12', '\x2A', '\x0', '\x5',
  '\xBF', 'G', '\xD4', '\xEF', '\xF7', '\x2F', '6', '\xEC',
  '\x12', '\x20', '\x1E', '\x8F', '\xD7', '\xAA', '\xD5', '\xEA',
  '\xAF', 'I', '5', 'F', '\xAA', 'T', '\x5F', '\x9F',
  '\x22', 'A', '\x2A', '\x95', '\xA', '\x83', '\xE5', 'r',
  '9', 'd', '\xB3', 'Y', '\x96', '\x99', 'L', '\x6',
  '\xE9', 't', '\x9A', '\x25', '\x85', '\x2C', '\xCB', 'T',
  '\xA7', '\xC4', 'b', '1', '\xB5', '\x5E', '\x0', '\x3',
  'h', '\x9A', '\xC6', '\x16', '\x82', '\x20', 'X', 'R',
  '\x14', 'E', '6', 'S', '\x94', '\xCB', 'e', 'x',
  '\xBD', '\x5E', '\xAA', 'U', 'T', '\x23', 'L', '\xC0',
  '\xE0', '\xE2', '\xC1', '\x8F', '\x0', '\x9E', '\xBC', '\x9',
  'A', '\x7C', '\x3E', '\x1F', '\x83', 'D', '\x22', '\x11',
  '\xD5', 'T', '\x40', '\x3F', '8', '\x80', 'w', '\xE5',
  '3', '\x7', '\xB8', '\x5C', '\x2E', 'H', '\x92', '\x4',
  '\x87', '\xC3', '\x81', '\x40', '\x20', '\x40', 'g', '\x98',
  '\xE9', '6', '\x1A', '\xA6', 'g', '\x15', '\x4', '\xE3',
  '\xD7', '\xC8', '\xBD', '\x15', '\xE1', 'i', '\xB7', 'C',
  '\xAB', '\xEA', 'x', '\x2F', 'j', 'X', '\x92', '\xBB',
  '\x18', '\x20', '\x9F', '\xCF', '3', '\xC3', '\xB8', '\xE9',
  'N', '\xA7', '\xD3', 'l', 'J', '\x0', 'i', '6',
  '\x7C', '\x8E', '\xE1', '\xFE', 'V', '\x84', '\xE7', '\x3C',
  '\x9F', 'r', '\x2B', '\x3A', 'B', '\x7B', '7', 'f',
  'w', '\xAE', '\x8E', '\xE', '\xF3', '\xBD', 'R', '\xA9',
  'd', '\x2', 'B', '\xAF', '\x85', '2', 'f', 'F',
  '\xBA', '\xC', '\xD9', '\x9F', '\x1D', '\x9A', 'l', '\x22',
  '\xE6', '\xC7', '\x3A', '\x2C', '\x80', '\xEF', '\xC1', '\x15',
  '\x90', '\x7', '\x93', '\xA2', '\x28', '\xA0', 'S', 'j',
  '\xB1', '\xB8', '\xDF', '\x29', '5', 'C', '\xE', '\x3F',
  'X', '\xFC', '\x98', '\xDA', 'y', 'j', 'P', '\x40',
  '\x0', '\x87', '\xAE', '\x1B', '\x17', 'B', '\xB4', '\x3A',
  '\x3F', '\xBE', 'y', '\xC7', '\xA', '\x26', '\xB6', '\xEE',
  '\xD9', '\x9A', '\x60', '\x14', '\x93', '\xDB', '\x8F', '\xD',
  '\xA', '\x2E', '\xE9', '\x23', '\x95', '\x29', 'X', '\x0',
  '\x27', '\xEB', 'n', 'V', 'p', '\xBC', '\xD6', '\xCB',
  '\xD6', 'G', '\xAB', '\x3D', 'l', '\x7D', '\xB8', '\xD2',
  '\xDD', '\xA0', '\x60', '\x83', '\xBA', '\xEF', '\x5F', '\xA4',
  '\xEA', '\xCC', '\x2', 'N', '\xAE', '\x5E', 'p', '\x1A',
  '\xEC', '\xB3', '\x40', '9', '\xAC', '\xFE', '\xF2', '\x91',
  '\x89', 'g', '\x91', '\x85', '\x21', '\xA8', '\x87', '\xB7',
  'X', '\x7E', '\x7E', '\x85', '\xBB', '\xCD', 'N', 'N',
  'b', 't', '\x40', '\xFA', '\x93', '\x89', '\xEC', '\x1E',
  '\xEC', '\x86', '\x2', 'H', '\x26', '\x93', '\xD0', 'u',
  '\x1D', '\x7F', '\x9', '2', '\x95', '\xBF', '\x1F', '\xDB',
  '\xD7', 'c', '\x8A', '\x1A', '\xF7', '\x5C', '\xC1', '\xFF',
  '\x22', 'J', '\xC3', '\x87', '\x0', '\x3', '\x0', 'K',
  '\xBB', '\xF8', '\xD6', '\x2A', 'v', '\x98', 'I', '\x0',
  '\x0', '\x0', '\x0', 'I', 'E', 'N', 'D', '\xAE',
  'B', '\x60', '\x82',
};
