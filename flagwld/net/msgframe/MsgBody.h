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


#ifndef FLAGWLD_NET_MSG_MSGBODY_H
#define FLAGWLD_NET_MSG_MSGBODY_H

#include <flagwld/net/msgframe/Message.h>

#include <flagwld/base/Types.h>
#include <flagwld/net/Buffer.h>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace flagwld
{
namespace net
{

class SockConnection;
typedef boost::shared_ptr<SockConnection> SockConnectionPtr;

class MsgBody: boost::noncopyable
{
 public:
  enum MsgBodyType{
    KFILEBODY, KMEMBODY
  };

  MsgBody();
  ~MsgBody();

  const char* data()
  { return body_.peek(); }

  size_t length()
  { return body_.readableBytes(); }

  void retrieve(size_t len)
  { body_.retrieve(len); }

  void retrieveAll()
  { body_.retrieveAll(); }

  size_t append(const char* buf, size_t len)
  {
    size_t l = body_.readableBytes() + len;

    if (l>=DEFAULT_MAX_MEM_BODY_LEN)
    {
      return -1;
    }

    body_.append(buf, len);

    return len;
  }

  size_t append(const string& str)
  {
    return append(str.c_str(), str.length());
  }

  string asString()
  {
    //return body_.retrieveAllAsString();
    return string(body_.peek(), body_.readableBytes());
  }

  size_t read(char* buf, size_t len);

  static void appendChunkToBuffer(Buffer* output, const char* buf, size_t len);
  static void appendChunkTrailerToBuffer(Buffer* output);

  /// Advanced interface
  inline Buffer* bodyBuffer()
  { return &body_; }

private:
  MsgBodyType type_;
  Buffer body_;
};

typedef boost::shared_ptr<MsgBody> MsgBodyPtr;

}
}
#endif //FLAGWLD_NET_MSG_MSGBODY_H
