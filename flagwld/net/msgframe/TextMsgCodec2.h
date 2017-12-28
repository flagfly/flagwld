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


#ifndef FLAGWLD_NET_MSG_MSGCONTEXT2_H
#define FLAGWLD_NET_MSG_MSGCONTEXT2_H

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

/****************************************** TextMsgRequestCodec2 ******************************************/
typedef boost::function<void (const SockConnectionPtr&, const MsgRequestPtr&, const MsgResponsePtr&)> MsgRequestCallback;
typedef boost::function<size_t (const SockConnectionPtr&, const MsgRequestPtr&, const MsgResponsePtr&, const char*d, size_t n)> MsgRequestBodyCallback;

class TextMsgRequestCodec2 : boost::noncopyable
{
 public:
  TextMsgRequestCodec2();
  ~TextMsgRequestCodec2();

  void setMsgRequestCallback(const MsgRequestCallback&cb)
  { msgRequestCallback_=cb; }

  void setMsgRequestBodyCallback(const MsgRequestBodyCallback&cb)
  { msgRequestBodyCallback_=cb; }

  inline void setHeaderOnly(bool on)
  { headerOnly_ = on; }

  // 0 bytes when connection reset by peer
  void onMessage(const SockConnectionPtr& conn,
                 Buffer* buf);

  void sendResponse(const flagwld::net::SockConnectionPtr& conn,
                          const flagwld::net::MsgRequestPtr& req,
                          const flagwld::net::MsgResponsePtr& resp);
 private:
  size_t parseExecute(const SockConnectionPtr& conn, const char*buff, size_t len);

  inline bool gotAll()
  { return parser_.gotAll(); }

  inline bool parseError()
  { return parser_.parserError(); }

  inline TextParser::MsgState getParseState()
  { return parser_.getParseState(); }

  inline bool connClose()
  { return request_->closeConnection(); }

  bool reset();

 private:
  inline void setProto(const char*s, const char*e)
  { request_->setProto(s, e); }
  inline void setMajor(unsigned int major)
  { request_->setMajor(major); }
  inline void setMinor(unsigned int minor)
  { request_->setMinor(minor); }
  inline void setMethod(const char*s, const char*e)
  { request_->setMethod(s, e); }
  inline void setPath(const char*s, const char*e)
  { request_->setPath(s, e); }
  inline void setQuery(const char*s, const char*e)
  { request_->setQuery(s, e); }
  inline void addHeader(const char*s, const char*c, const char*v)
  { request_->addHeader(s, c, v); }
  inline size_t appendBody(const SockConnectionPtr& conn, const char*d, size_t n)
  {
    if (msgRequestBodyCallback_)
    {
      return msgRequestBodyCallback_(conn, request_, response_, d, n);
    }
    return request_->appendBody(d, n);
  }

 private:
  MsgRequestCallback msgRequestCallback_;
  MsgRequestBodyCallback msgRequestBodyCallback_;
  TextParser parser_;
  MsgRequestPtr request_;
  MsgResponsePtr response_;
  bool headerOnly_;
};

/****************************************** TextMsgResponseCodec2 ******************************************/
typedef boost::function<void (const SockConnectionPtr&, const MsgResponsePtr&)> MsgResponseCallback;
typedef boost::function<size_t (const SockConnectionPtr&, const MsgResponsePtr&, const char*d, size_t n)> MsgResponseBodyCallback;

class TextMsgResponseCodec2 : boost::noncopyable
{
 public:
  TextMsgResponseCodec2();
  ~TextMsgResponseCodec2();

  void setMsgResponseCallback(const MsgResponseCallback&cb)
  { msgResponseCallback_=cb; }

  void setMsgResponseBodyCallback(const MsgResponseBodyCallback&cb)
  { msgResponseBodyCallback_=cb; }

  inline void setHeaderOnly(bool on)
  { headerOnly_ = on; }

  // 0 bytes when connection reset by peer
  void onMessage(const SockConnectionPtr& conn,
                 Buffer* buf);

 private:
  size_t parseExecute(const SockConnectionPtr& conn, const char*buff, size_t len);

  inline bool connClose()
  { return response_->closeConnection(); }

  inline bool parseError()
  { return parser_.parserError(); }

  inline TextParser::MsgState getParseState()
  { return parser_.getParseState(); }

  inline bool gotAll()
  { return parser_.gotAll(); }

  bool reset();

 private:
  inline void setProto(const char*s, const char*e)
  { response_->setProto(s, e); }
  inline void setMajor(unsigned int major)
  { response_->setMajor(major); }
  inline void setMinor(unsigned int minor)
  { response_->setMinor(minor); }
  inline void setCode(int c)
  { response_->setCode(c); }
  void setPhrase(const char*s, const char*e)
  { response_->setPhrase(s, e); }
  inline void addHeader(const char*s, const char*c, const char*v)
  { response_->addHeader(s, c, v); }
  inline size_t appendBody(const SockConnectionPtr& conn, const char*d, size_t n)
  {
    if (msgResponseBodyCallback_)
    {
      return msgResponseBodyCallback_(conn, response_, d, n);
    }
    return response_->appendBody(d, n);
  }

 private:
  MsgResponseCallback msgResponseCallback_;
  MsgResponseBodyCallback msgResponseBodyCallback_;
  TextParser parser_;
  MsgResponsePtr response_;
  bool headerOnly_;
};

}
}

#endif  // FLAGWLD_NET_MSG_MSGCONTEXT2_H
