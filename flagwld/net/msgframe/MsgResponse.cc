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


#include <flagwld/base/Logging.h>
#include <flagwld/net/msgframe/MsgResponse.h>
#include <flagwld/net/msgframe/MsgBody.h>
#include <flagwld/net/Buffer.h>
#include <flagwld/net/SockConnection.h>

#include <stdio.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#undef __STDC_FORMAT_MACROS

using namespace flagwld;
using namespace flagwld::net;

void MsgResponse::setBody(MsgBody* body)
{
  body_.reset(body);
  if (body_)
  {
    nbody_ = body_->length();
  }
  else
  {
    nbody_ = 0;
  }
}

void MsgResponse::setBody(const MsgBodyPtr& body)
{
  body_=body;
  if (body_)
  {
    nbody_ = body_->length();
  }
  else
  {
    nbody_ = 0;
  }
}

void MsgResponse::swapBody(MsgBodyPtr& body)
{
  body_.swap(body);
  if (body_)
  {
    nbody_ = body_->length();
  }
  else
  {
    nbody_ = 0;
  }
}

size_t MsgResponse::appendBody( const char* p, size_t len )
{
  assert( body_ );
  nbody_ += len;
  return body_->append( p, len );
}

void MsgResponse::swap(MsgResponse& that)
{
  proto_.swap(that.proto_);
  major_ = that.major_;
  minor_ = that.minor_;
  code_ = that.code_;
  phrase_.swap(that.phrase_);
  timestamp_.swap(that.timestamp_);
  nmessage_=that.nmessage_;
  nbody_=that.nbody_;
  nparsed_=that.nparsed_;
  headers_.swap(that.headers_);
  body_.swap(that.body_);
  connectionUpgrade_=that.connectionUpgrade_;
  closeConnection_=that.closeConnection_;
  chunked_=that.chunked_;
}

void MsgResponse::reset()
{
  proto_.clear();
  major_=1;
  minor_=0;
  code_=0;
  phrase_.clear();
  timestamp_ = Timestamp::now();
  nmessage_=0;
  nbody_=0;
  nparsed_=0;
  headers_.clear();
  body_.reset();
  connectionUpgrade_=false;
  closeConnection_=true;
  chunked_=false;
}

void MsgResponse::appendToBuffer(Buffer* output) const
{
  char buf[256] = {0};
  snprintf(buf, sizeof buf, "%s/%d.%d %d %s\r\n", proto().c_str(), major(), minor(), code(), phrase().c_str());
  output->append(buf);

  if (connectionUpgrade_)
  {
    output->append("Connection: Upgrade\r\n");
  }
  else if (closeConnection_)
  {
    output->append("Connection: close\r\n");
  }
  else
  {
    output->append("Connection: Keep-Alive\r\n");
  }
  
  if (chunked_)
  {
    bzero(buf,sizeof(buf));
    snprintf(buf, sizeof buf, "Transfer-Encoding: chunked\r\n");
    output->append(buf);
  }
  else
  {
    bzero(buf,sizeof(buf));
    snprintf(buf, sizeof buf, "Content-Length: %zd\r\n", nbody_);
    output->append(buf);
  }

  for (HeaderUmap::const_iterator it = headers_.begin();
       it != headers_.end();
       ++it)
  {
    output->append(it->first);
    output->append(": ");
    output->append(it->second);
    output->append("\r\n");
  }

  output->append("\r\n");

  if (body_)
  {
    if (chunked_)
    {
       MsgBody::appendChunkToBuffer(output, body_->data(), body_->length());
       MsgBody::appendChunkTrailerToBuffer(output);
    }
    else
    {
       output->append(body_->data(), body_->length());
    }
  }
}

void MsgResponse::send(const SockConnectionPtr& conn)
{
  Buffer buff;
  appendToBuffer(&buff);
  conn->send(&buff);
}

void MsgResponse::sendBody(const SockConnectionPtr& conn, const char* buf, size_t n)
{
  assert(!body_);

  if (chunked_)
  {
    if (n>0)
    {
      Buffer buff;
      MsgBody::appendChunkToBuffer(&buff, buf, n);
      conn->send(&buff);

      nparsed_ += buff.readableBytes();

      nbody_ += n;
    }
    else
    {
      Buffer buff;
      MsgBody::appendChunkTrailerToBuffer(&buff);
      conn->send(&buff);

      nparsed_ += buff.readableBytes();
    }
  }
  else
  {
    if (n>0)
    {
      conn->send(buf, static_cast<int>(n));
      nparsed_ += n;
    }
    else
    {
      assert (nparsed_ == nbody_);

      if (nparsed_ != nbody_)
      {
        LOG_WARN;
      }
    }
  }
}

