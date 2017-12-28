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


#ifndef FLAGWLD_NET_MSG_MSGCONTEXT_H
#define FLAGWLD_NET_MSG_MSGCONTEXT_H

#include <flagwld/net/SockConnection.h>
#include <flagwld/net/msgframe/MsgRequest.h>
#include <flagwld/net/msgframe/MsgResponse.h>
#include <flagwld/net/msgframe/TextMsgParser.h>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace flagwld
{
namespace net
{

class MsgRequest;
typedef boost::shared_ptr<MsgRequest> MsgRequestPtr;
class MsgResponse;
typedef boost::shared_ptr<MsgResponse> MsgResponsePtr;

/****************************************** TextMsgServerContext ******************************************/
typedef boost::function<void (const SockConnectionPtr&, const MsgRequestPtr&, const MsgResponsePtr&)> MsgRequestCallback;
typedef boost::function<void (const SockConnectionPtr&, const MsgRequestPtr&, const MsgResponsePtr&)> LogMsgRequestCallback;

class TextMsgServerContext : boost::noncopyable
{
 public:
  TextMsgServerContext();
  ~TextMsgServerContext();

  void onRequest(const SockConnectionPtr& conn,
                         const MsgRequestPtr& req,
                         const MsgResponsePtr& resp);

  void setRequestCallback(const flagwld::string& uri,
                         const MsgRequestCallback&cb);

  void setLogRequestCallback(const LogMsgRequestCallback&cb)
  { logCallback_ = cb; }

  void sendResponse(const flagwld::net::SockConnectionPtr& conn,
                          const flagwld::net::MsgRequestPtr& req,
                          const flagwld::net::MsgResponsePtr& resp);

 private:
  boost::unordered_map<flagwld::string, MsgRequestCallback> callbacks_;
  LogMsgRequestCallback logCallback_;
};

/****************************************** TextMsgClientContext ******************************************/
typedef boost::function<void (const SockConnectionPtr&, const MsgResponsePtr&)> MsgResponseCallback;
typedef boost::function<size_t (const SockConnectionPtr&, const MsgResponsePtr&, const char*d, size_t n)> MsgResponseBodyCallback;

class TextMsgClientContext : boost::noncopyable
{
 public:
  TextMsgClientContext();
  ~TextMsgClientContext();

  void onResponse(const SockConnectionPtr& conn, const MsgResponsePtr&resp)
  {
    Buffer buf;
    resp->appendToBuffer(&buf);
    std::cout << "[" << buf.peek() << resp->getBody()->asString() << "]" << std::endl;

    conn->forceClose();
    gloop->quit();
  }

  void setRequestCallback(const flagwld::string& uri,
                         const MsgRequestCallback&cb);

  void setLogRequestCallback(const LogMsgRequestCallback&cb)
  { logCallback_ = cb; }

  void setMsgResponseBodyCallback(const MsgResponseBodyCallback&cb)
  { msgResponseBodyCallback_=cb; }


 private:

};

}
}

#endif  // FLAGWLD_NET_MSG_MSGCONTEXT_H
