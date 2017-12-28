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

void defaultRequestMsgCallback(const SockConnectionPtr& conn, const MsgRequestPtr& req, const MsgResponsePtr& resp)
{
  if (req->getParseMsgState() == ParseMsgFinalE)
  {
    resp->setCode(404);
    resp->setPhrase("Not Found");
    resp->setCloseConnection(true);
    resp->send(conn);
  }
}

void defaultMsgResponseCallback(const SockConnectionPtr& conn, const MsgResponsePtr& resp)
{
  if (resp->getParseMsgState() == ParseMsgFinalE)
  {
    LOG_TRACE << "code[" << resp->code() << "] msg[" << resp->phrase() << "]";
  }
}


}
}
}

/****************************************** TextMsgRequestCodec ******************************************/

TextMsgRequestCodec::TextMsgRequestCodec()
               :msgRequestCallback_(detail::defaultRequestMsgCallback),
                parser_(TextParser::MsgRequestMode),
                request_(new MsgRequest),
                response_(new MsgResponse(false)),
                headerOnly_(false)
                           
{
  LOG_TRACE << "TextMsgRequestCodec::ctor[-] at " << this;

  assert (request_);

  parser_.setProtoCallback( boost::bind(&TextMsgRequestCodec::setProto, this, _1, _2) );
  parser_.setMajorCallback( boost::bind(&TextMsgRequestCodec::setMajor, this, _1) );
  parser_.setMinorCallback( boost::bind(&TextMsgRequestCodec::setMinor, this, _1) );
    
  parser_.setMethodCallback( boost::bind(&TextMsgRequestCodec::setMethod, this, _1, _2) );
  parser_.setPathCallback( boost::bind(&TextMsgRequestCodec::setPath, this, _1, _2) );
  parser_.setQueryCallback( boost::bind(&TextMsgRequestCodec::setQuery, this, _1, _2) );
    
  parser_.setHeaderCallback( boost::bind(&TextMsgRequestCodec::addHeader, this, _1, _2, _3) );
  parser_.setBodyCallback( boost::bind(&TextMsgRequestCodec::appendBody, this, _1, _2, _3) );
}

TextMsgRequestCodec::~TextMsgRequestCodec()
{
  LOG_TRACE << "TextMsgRequestCodec::dtor[-] at " << this;
}

bool TextMsgRequestCodec::reset()
{
  parser_.reset();
  request_.reset(new MsgRequest);
  response_.reset(new MsgResponse(false));

  return (request_&&response_);
}

void TextMsgRequestCodec::onMessage(const SockConnectionPtr& conn,
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

      if (response_->closeConnection()&&!response_->ChunkedContent())
      { 
        LOG_TRACE << conn->name() << " shutdown from " << conn->peerAddress().toString() << " closed=" << response_->closeConnection() << " code=" << response_->code();
        reset();
        conn->shutdown();
      }
      else
      {
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
}

size_t TextMsgRequestCodec::parseExecute(const SockConnectionPtr& conn, const char*buff, size_t len)
{
  size_t nparsed = 0;

__reexecute_byte:
  //LOG_TRACE << "TextMsgRequestCodec::parseExecute[" << buff <<"]["<< len <<"] nparsed=" << nparsed;
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

/****************************************** TextMsgResponseCodec ******************************************/

TextMsgResponseCodec::TextMsgResponseCodec()
                      :msgResponseCallback_(detail::defaultMsgResponseCallback),
                       parser_(TextParser::MsgResponseMode),
                       response_(new MsgResponse(false)),
                       headerOnly_(false)
{           
  LOG_TRACE << "TextMsgResponseCodec::ctor[-] at " << this;

  parser_.setProtoCallback( boost::bind(&TextMsgResponseCodec::setProto, this, _1, _2) );
  parser_.setMajorCallback( boost::bind(&TextMsgResponseCodec::setMajor, this, _1) );
  parser_.setMinorCallback( boost::bind(&TextMsgResponseCodec::setMinor, this, _1) );
    
  parser_.setCodeCallback( boost::bind(&TextMsgResponseCodec::setCode, this, _1) );
  parser_.setPhraseCallback( boost::bind(&TextMsgResponseCodec::setPhrase, this, _1, _2) );
    
  parser_.setHeaderCallback( boost::bind(&TextMsgResponseCodec::addHeader, this, _1, _2, _3) );
  parser_.setBodyCallback( boost::bind(&TextMsgResponseCodec::appendBody, this, _1, _2, _3) );
}             
TextMsgResponseCodec::~TextMsgResponseCodec()
{               
  LOG_TRACE << "TextMsgResponseCodec::dtor[-] at " << this;
}             

bool TextMsgResponseCodec::reset()
{
  parser_.reset();
  response_.reset(new MsgResponse(false));

  return response_;
}

void TextMsgResponseCodec::onMessage(const SockConnectionPtr& conn,
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

      bool connclose = connClose();
      LOG_TRACE << conn->name() << " Connection-Close: " << connclose;

      response_->setnMessage(parser_.nmessage());
      response_->setnBody(parser_.nbody());
      response_->setnParsed(parser_.nread());
      response_->setTimestamp(Timestamp::now());

      msgResponseCallback_(conn, response_);

      if (connclose)
      {
        LOG_TRACE << conn->name() << " shutdown from " << conn->peerAddress().toString();
        reset();
        conn->shutdown();
      }
      else
      {
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
}
              
size_t TextMsgResponseCodec::parseExecute(const SockConnectionPtr& conn, const char*buff, size_t len)
{               
  size_t nparsed = 0;

__reexecute_byte:
  //LOG_TRACE << "TextMsgResponseCodec::parseExecute[" << buff+nparsed <<"]["<< len-nparsed <<"] nparsed=" << nparsed;
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
