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


#ifndef FLAGWLD_NET_MSG_MSGRESPONSE_H
#define FLAGWLD_NET_MSG_MSGRESPONSE_H

#include <flagwld/net/msgframe/Message.h>

#include <flagwld/base/Timestamp.h>
#include <flagwld/base/Types.h>
#include <flagwld/base/StringPiece.h>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include <string.h>//strlen

namespace flagwld
{
namespace net
{

class Buffer;
class SockConnection;
typedef boost::shared_ptr<SockConnection> SockConnectionPtr;
class MsgBody;
typedef boost::shared_ptr<MsgBody> MsgBodyPtr;

class MsgResponse;
typedef boost::shared_ptr<MsgResponse> MsgResponsePtr;

typedef boost::unordered_map<string, string> HeaderUmap;

class MsgResponse : boost::noncopyable
{
 public:
  explicit MsgResponse(bool close)
    : major_(1),
      minor_(0),
      code_(0),
      timestamp_(Timestamp::now()),
      nmessage_(0),
      nbody_(0),
      nparsed_(0),
      connectionUpgrade_(false),
      closeConnection_(close),
      chunked_(false),
      parseState_(ParseMsgBeginE)
  {
  }

  ~MsgResponse()
  {
  }

  void setProto(const char* start, const char* end)
  { proto_.assign(start, end); }

  void setProto(const string& p)
  { proto_=p; }

  const string& proto() const
  { return proto_; }

  inline void setMajor(unsigned int v)
  { major_ = v; }

  inline unsigned int major() const
  { return major_; }

  inline void setMinor(unsigned int v)
  { minor_ = v; }

  inline unsigned int minor() const
  { return minor_; }

  void setCode(int c)
  { code_ = c; }

  int code() const
  { return code_; }

  void setMessage(const char* start, const char* end)  __attribute__((deprecated))
  { phrase_.assign(start, end); }

  void setMessage(const string& m)  __attribute__((deprecated))
  { phrase_ = m; }

  string message() const  __attribute__((deprecated))
  { return phrase_; }

  void setPhrase(const char* start, const char* end)
  { phrase_.assign(start, end); }

  void setPhrase(const string& m)
  { phrase_ = m; }

  string phrase() const
  { return phrase_; }

  void setTimestamp(Timestamp t)
  { timestamp_ = t; }

  Timestamp timestamp() const
  { return timestamp_; }

  void setConnectionUpgrade(bool on)
  { connectionUpgrade_ = on; }

  bool connectionUpgrade() const
  { return connectionUpgrade_; }

  void setCloseConnection(bool on)
  { closeConnection_ = on; }

  bool closeConnection() const
  { return closeConnection_; }

  void setChunkedContent(bool on)
  { chunked_ = on; }

  bool ChunkedContent() const
  { return chunked_; }

  void setContentType(const string& contentType)
  { addHeader("Content-Type", contentType); }

  // FIXME: replace string with StringPiece
  void addHeader(const string& key, const string& value)
  { headers_.insert(HeaderUmap::value_type(key,value)); }

  void addHeader(const char* start, const char* colon, const char* end)
  {
    string field(start, colon);
    ++colon;
    while (colon < end && isspace(*colon))
    {
      ++colon;
    }
    string value(colon, end);
    while (!value.empty() && isspace(value[value.size()-1]))
    {
      value.resize(value.size()-1);
    }
    headers_.insert(HeaderUmap::value_type(field,value));
  }

  string getHeader(const string& field) const
  {
    string result;
    HeaderUmap::const_iterator it = headers_.find(field);
    if (it != headers_.end())
    {
      result = it->second;
    }
    return result;
  }

  void eraseHeader(const string& field) 
  {
    headers_.erase(field);
  }

  const HeaderUmap& headers() const
  { return headers_; }

  void setBody(MsgBody* body);
  
  void setBody(const MsgBodyPtr& body);
  void swapBody(MsgBodyPtr& body);
  
  size_t appendBody( const char* p, size_t len );
  
  size_t appendBody( const char* str)
  { return appendBody(str, strlen(str)); }
  
  size_t appendBody( const string &body)
  { return appendBody(body.c_str(), body.length()); }

  const MsgBodyPtr& getBody() const
  { return body_; }

  void swap(MsgResponse& that);

  void reset();

  void appendToBuffer(Buffer* output) const;

  inline size_t nmessage() const { return nmessage_; }
  inline size_t nbody() const { return nbody_; }
  inline size_t nparsed() const { return nparsed_; }

  inline void setParseMsgState(ParseMsgState s)
  { parseState_ = s; }

  inline ParseMsgState getParseMsgState()
  { return parseState_; }

  void send(const SockConnectionPtr& conn);
  // 0 for end of body
  void sendBody(const SockConnectionPtr& conn, const char* buf, size_t n);

 private:
  friend class TextMsgResponseCodec;
  friend class TextMsgResponseCodec2;

  inline void setnMessage(size_t n) { nmessage_=n; }
  inline void setnBody(size_t n) { nbody_=n; }
  inline void setnParsed(size_t n) { nparsed_=n; }

 private:
  string proto_;
  unsigned int major_;
  unsigned int minor_;
  int code_;
  string phrase_;
  Timestamp timestamp_;

  size_t nmessage_;// all message header parsed bytes by parser
  size_t nbody_;//all body parserd bytes by parser
  size_t nparsed_;//all parsed bytes by parser. or !chunked, all body send, when use sendBody.

  HeaderUmap headers_;

  bool connectionUpgrade_;
  bool closeConnection_;
  bool chunked_;

  MsgBodyPtr body_;

  ParseMsgState parseState_;
};

}
}

#endif  // FLAGWLD_NET_MSG_MSGRESPONSE_H
