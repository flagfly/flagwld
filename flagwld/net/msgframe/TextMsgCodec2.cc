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


#include <flagwld/net/msgframe/TextMsgCodec2.h>

#include <flagwld/net/msgframe/MsgBody.h>

#include <flagwld/base/Logging.h>

#include <boost/bind.hpp>

#include <stdio.h>

using namespace flagwld;
using namespace flagwld::net;

namespace flagwld
{
namespace net
{
namespace detail
{

void defaultRequestMsgCallback2(const SockConnectionPtr& conn, const MsgRequestPtr& req, const MsgResponsePtr& resp)
{
  if (req->getParseMsgState() == ParseMsgFinalE)
  {
    resp->setCode(404);
    resp->setPhrase("Not Found");
    resp->setCloseConnection(true);
    resp->send(conn);
  }
}

void defaultMsgResponseCallback2(const SockConnectionPtr& conn, const MsgResponsePtr& resp)
{
  if (resp->getParseMsgState() == ParseMsgFinalE)
  {
    LOG_TRACE << "code[" << resp->code() << "] msg[" << resp->phrase() << "]";
  }
}


}
}
}

/****************************************** TextMsgRequestCodec2 ******************************************/

TextMsgRequestCodec2::TextMsgRequestCodec2()
               :msgRequestCallback_(detail::defaultRequestMsgCallback2),
                parser_(TextParser::MsgRequestMode),
                request_(new MsgRequest),
                response_(new MsgResponse(false)),
                headerOnly_(false)
                           
{
  LOG_TRACE << "TextMsgRequestCodec2::ctor[-] at " << this;

  assert (request_);

  parser_.setProtoCallback( boost::bind(&TextMsgRequestCodec2::setProto, this, _1, _2) );
  parser_.setMajorCallback( boost::bind(&TextMsgRequestCodec2::setMajor, this, _1) );
  parser_.setMinorCallback( boost::bind(&TextMsgRequestCodec2::setMinor, this, _1) );
    
  parser_.setMethodCallback( boost::bind(&TextMsgRequestCodec2::setMethod, this, _1, _2) );
  parser_.setPathCallback( boost::bind(&TextMsgRequestCodec2::setPath, this, _1, _2) );
  parser_.setQueryCallback( boost::bind(&TextMsgRequestCodec2::setQuery, this, _1, _2) );
    
  parser_.setHeaderCallback( boost::bind(&TextMsgRequestCodec2::addHeader, this, _1, _2, _3) );
  parser_.setBodyCallback( boost::bind(&TextMsgRequestCodec2::appendBody, this, _1, _2, _3) );
}

TextMsgRequestCodec2::~TextMsgRequestCodec2()
{
  LOG_TRACE << "TextMsgRequestCodec2::dtor[-] at " << this;
}

bool TextMsgRequestCodec2::reset()
{
  parser_.reset();
  request_.reset(new MsgRequest);
  response_.reset(new MsgResponse(false));

  return (request_&&response_);
}

void TextMsgRequestCodec2::onMessage(const SockConnectionPtr& conn,
                           Buffer* buf)
{
  size_t nparsed = 0;

  if (buf->readableBytes()==0)
  {
    parser_.eof(); 
  }

__reexecute_byte:
  nparsed = parseExecute( conn, buf->peek(), buf->readableBytes() );

  //LOG_TRACE << "[" << buf->peek() <<"]["<<buf->readableBytes()<<"] nparsed=" << nparsed;
  
  if (nparsed>0)
  { 
    buf->retrieve(nparsed);
    nparsed = 0;
  }
  
  if (parseError())
  { 
    if (getParseState()!=TextParser::SysError)
    {
      LOG_WARN << "Peer: " << conn->name() << " Parse Error";
    }
    else
    {
      LOG_WARN << "Peer: " << conn->name() << " Parse Body Sys Error";
    }
    //conn->shutdown();
    conn->forceClose();
  }
  else
  { 
    if (gotAll())
    {
      request_->setParseMsgState(ParseMsgFinalE);

      request_->setnMessage(parser_.nmessage());
      request_->setnBody(parser_.nbody());
      request_->setnParsed(parser_.nread());
      request_->setTimestamp(Timestamp::now());

      msgRequestCallback_(conn, request_, response_);

      if (reset())
      {
        if (buf->readableBytes()>0)
        { 
          goto __reexecute_byte;
        }
      }
      else
      {
        conn->shutdown();
      }
    }
  }
}

size_t TextMsgRequestCodec2::parseExecute(const SockConnectionPtr& conn, const char*buff, size_t len)
{
  size_t nparsed = 0;

__reexecute_byte:
  //LOG_TRACE << "TextMsgRequestCodec2::parseExecute[" << buff <<"]["<< len <<"] nparsed=" << nparsed;
  nparsed += parser_.parseExecute(conn, buff+nparsed, len-nparsed);

  if (parser_.gotMessage())
  {
    request_->setParseMsgState(ParseMsgHeaderE);

    const string& connection = request_->getHeader("Connection");
    if (connection == "Upgrade")
    {
      request_->setCloseConnection(false);
      response_->setCloseConnection(false);
      request_->setConnectionUpgrade(true);
      response_->setConnectionUpgrade(true);
    }
    else
    {
      bool close = connection == "close" ||
                     ((1==request_->major()&&0==request_->minor()) && connection != "Keep-Alive");
      request_->setCloseConnection(close);
      response_->setCloseConnection(close);
    }
    request_->eraseHeader("Connection");

    bool chunked = ( request_->getHeader("Transfer-Encoding") == "chunked" );

    if (chunked)
    {
      request_->eraseHeader("Transfer-Encoding");
    }

    request_->setChunkedContent(chunked);
  
    if (!headerOnly_) // body size default 0
    {
      parser_.setChunkedContent(chunked);

      if (chunked)
      {
        request_->setBody( new MsgBody() );
        LOG_TRACE << "Body Size: chunked";
      }
      else
      {
        const string& contLen = request_->getHeader("Content-Length");
  
        if (contLen.length()>0)
        {
          uint64_t bsize = 0;
          bsize = strtoll(contLen.c_str(),NULL,10);
          if (bsize>0)
          {
            parser_.setContentLengh(bsize);
            request_->setBody( new MsgBody() );
          }
   
          request_->eraseHeader("Content-Length");
  
          LOG_TRACE << "Body Size: " << bsize;
        }
        else
        {
      //does not fully compatible with rfc 1945 7.2.2
          request_->setCloseConnection(true);
          response_->setCloseConnection(true);
        }
      }
    }

    if (msgRequestBodyCallback_)
    {
      msgRequestCallback_(conn, request_, response_);
    }

    request_->setParseMsgState(ParseMsgBodyE);
    goto __reexecute_byte;
  }

  return nparsed;
}

/****************************************** TextMsgResponseCodec2 ******************************************/

TextMsgResponseCodec2::TextMsgResponseCodec2()
                      :msgResponseCallback_(detail::defaultMsgResponseCallback2),
                       parser_(TextParser::MsgResponseMode),
                       response_(new MsgResponse(false)),
                       headerOnly_(false)
{           
  LOG_TRACE << "TextMsgResponseCodec2::ctor[-] at " << this;

  parser_.setProtoCallback( boost::bind(&TextMsgResponseCodec2::setProto, this, _1, _2) );
  parser_.setMajorCallback( boost::bind(&TextMsgResponseCodec2::setMajor, this, _1) );
  parser_.setMinorCallback( boost::bind(&TextMsgResponseCodec2::setMinor, this, _1) );
    
  parser_.setCodeCallback( boost::bind(&TextMsgResponseCodec2::setCode, this, _1) );
  parser_.setPhraseCallback( boost::bind(&TextMsgResponseCodec2::setPhrase, this, _1, _2) );
    
  parser_.setHeaderCallback( boost::bind(&TextMsgResponseCodec2::addHeader, this, _1, _2, _3) );
  parser_.setBodyCallback( boost::bind(&TextMsgResponseCodec2::appendBody, this, _1, _2, _3) );
}             
TextMsgResponseCodec2::~TextMsgResponseCodec2()
{               
  LOG_TRACE << "TextMsgResponseCodec2::dtor[-] at " << this;
}             

bool TextMsgResponseCodec2::reset()
{
  parser_.reset();
  response_.reset(new MsgResponse(false));

  return response_;
}

void TextMsgResponseCodec2::onMessage(const SockConnectionPtr& conn,
                           Buffer* buf)
{
  size_t nparsed = 0;

  if (buf->readableBytes()==0)
  {
    parser_.eof();
  }

__reexecute_byte:
  nparsed = parseExecute( conn, buf->peek(), buf->readableBytes() );

  LOG_TRACE << "[" << string(buf->peek(), 128) <<"]["<<buf->readableBytes()<<"] nparsed=" << nparsed;
  
  if (nparsed>0)
  {
    buf->retrieve(nparsed);
    nparsed = 0;
  }
  
  if (parseError())
  {
    if (getParseState()!=TextParser::SysError)
    {
      LOG_WARN << "Peer: " << conn->name() << " Parse Error";
    }
    else
    {
      LOG_WARN << "Peer: " << conn->name() << " Parse Body Sys Error";
    }
    //conn->shutdown();
    conn->forceClose();
  }
  else 
  {
    if (gotAll())
    {
      response_->setParseMsgState(ParseMsgFinalE);

      response_->setnMessage(parser_.nmessage());
      response_->setnBody(parser_.nbody());
      response_->setnParsed(parser_.nread());
      response_->setTimestamp(Timestamp::now());

      msgResponseCallback_(conn, response_);

      if (reset())
      {
        if (buf->readableBytes()>0)
        {
          goto __reexecute_byte;
        }
      }
      else
      {
        conn->shutdown();
      }
    }
  }
}
              
size_t TextMsgResponseCodec2::parseExecute(const SockConnectionPtr& conn, const char*buff, size_t len)
{               
  size_t nparsed = 0;

__reexecute_byte:
  //LOG_TRACE << "TextMsgResponseCodec2::parseExecute[" << buff+nparsed <<"]["<< len-nparsed <<"] nparsed=" << nparsed;
  nparsed += parser_.parseExecute(conn, buff+nparsed, len-nparsed);

  if (parser_.gotMessage())
  {
    response_->setParseMsgState(ParseMsgHeaderE);

    const string& connection = response_->getHeader("Connection");

    if (connection == "Upgrade")
    {
      response_->setCloseConnection(false);
      response_->setConnectionUpgrade(true);
    }
    else
    {
      bool close = connection == "close" ||
                     ((1==response_->major()&&0==response_->minor()) && connection != "Keep-Alive");

      response_->setCloseConnection(close);
    }

    response_->eraseHeader("Connection");

    bool chunked = ( response_->getHeader("Transfer-Encoding") == "chunked" );

    if (chunked)
    {
      response_->eraseHeader("Transfer-Encoding");
    }

    response_->setChunkedContent(chunked);
  
    if (!headerOnly_) // body size default 0
    {
      parser_.setChunkedContent(chunked);
  
      if (chunked)
      {
        response_->setBody( new MsgBody() );
      }
      else
      {
        const string& contLen = response_->getHeader("Content-Length");
  
        if (contLen.length()>0)
        {
          uint64_t bsize = 0;
          bsize = strtoll(contLen.c_str(),NULL,10);
          if (bsize>0)
          {
            parser_.setContentLengh(bsize);
            response_->setBody( new MsgBody() );
          }
          response_->eraseHeader("Content-Length");
  
          LOG_TRACE << "Body Size: " << bsize;
        }
        else
        {
          if (!response_->connectionUpgrade())
          {
            parser_.setContentLengh(-1);
            response_->setBody( new MsgBody() );
            LOG_TRACE << "Body Size: unknown";
          }
        }
      }
    }

    if (msgResponseBodyCallback_)
    {
      msgResponseCallback_(conn, response_);
    }

    response_->setParseMsgState(ParseMsgBodyE);
    goto __reexecute_byte;
  }

  return nparsed;
}

