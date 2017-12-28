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


#ifndef __FLAGWLD_NET_MSG_TEXTPARSER_H_
#define __FLAGWLD_NET_MSG_TEXTPARSER_H_

#include <flagwld/net/msgframe/Message.h>
#include <flagwld/net/msgframe/TextLineParser.h>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

// when chunk , ignore chunk-extension and trailer entity-header

namespace flagwld
{
namespace net
{

class SockConnection;
typedef boost::shared_ptr<SockConnection> SockConnectionPtr;

void defaultProtoCallback(const char*, const char*);
void defaultMajorCallback(unsigned int);
void defaultMinorCallback(unsigned int);
void defaultMethodCallback(const char*, const char*);
void defaultPathCallback(const char*, const char*);
void defaultQueryCallback(const char*, const char*);
void defaultCodeCallback(int);
void defaultPhraseCallback(const char*, const char*);
void defaultHeaderCallback(const char*, const char*, const char*);
size_t defaultBodyCallback(const SockConnectionPtr& conn, const char*, size_t);

class TextLineParser;
class TextParser : boost::noncopyable
{
public:
  enum MsgMode
  {
    MsgUnknownMode=-1,
    MsgRequestMode,
    MsgResponseMode,
  };
  
  enum MsgState
  {
    SysError=-1,
    MsgError,
    FirstLine,
    HeaderLine,
    HeaderAll,
    MsgBody,
    MsgAll,
  };
  
  enum ChunkState
  {
    ChunkError=-1,
    ChunkHeaderLine,
    ChunkData,
    ChunkEndLine,
    ChunkTrailer,
  };

public:
  typedef boost::function<void(const char*, const char*)> ProtoCallback;  
  typedef boost::function<void(unsigned int)> MajorCallback;  
  typedef boost::function<void(unsigned int)> MinorCallback;  

  //for request
  typedef boost::function<void(const char*, const char*)> MethodCallback;  
  typedef boost::function<void(const char*, const char*)> PathCallback;  
  typedef boost::function<void(const char*, const char*)> QueryCallback;  
  
  //for response
  typedef boost::function<void(int)> CodeCallback;  
  typedef boost::function<void(const char*, const char*)> PhraseCallback;  


  typedef boost::function<void(const char*, const char*, const char*)> HeaderCallback;  
  typedef boost::function<size_t(const SockConnectionPtr& conn, const char*, size_t)> BodyCallback;  

  public:
  TextParser(MsgMode m);
  ~TextParser();

  void setProtoCallback(const ProtoCallback& cb) { protoCallback_ = cb; }
  void setMajorCallback(const MajorCallback& cb) { majorCallback_ = cb; }
  void setMinorCallback(const MinorCallback& cb) { minorCallback_ = cb; }

  void setMethodCallback(const MethodCallback& cb) { methodCallback_ = cb; }
  void setPathCallback(const PathCallback& cb) { pathCallback_ = cb; }
  void setQueryCallback(const QueryCallback& cb) { queryCallback_ = cb; }

  void setCodeCallback(const CodeCallback& cb) { codeCallback_ = cb; }
  void setPhraseCallback(const PhraseCallback& cb) { phraseCallback_ = cb; }

  void setHeaderCallback(const HeaderCallback& cb) { headerCallback_ = cb; }
  void setBodyCallback(const BodyCallback& cb) { bodyCallback_ = cb; }

#ifdef __GXX_EXPERIMENTAL_CXX0X__
  void setFirstLineCallback(FirstLineCallback&& cb) { firstLineCallback_ = cb; }
  void setHeaderLineCallback(HeaderLineCallback&& cb) { headerLineCallback_ = cb; }
  void setBodyCallback(BodyCallback&& cb) { bodyCallback_ = cb; }

  void setProtoCallback(ProtoCallback&& cb) { protoCallback_ = cb; }
  void setMajorCallback(MajorCallback&& cb) { majorCallback_ = cb; }
  void setMinorCallback(MinorCallback&& cb) { minorCallback_ = cb; }

  void setMethodCallback(MethodCallback&& cb) { methodCallback_ = cb; }
  void setPathCallback(PathCallback&& cb) { pathCallback_ = cb; }
  void setQueryCallback(QueryCallback&& cb) { queryCallback_ = cb; }

  void setCodeCallback(CodeCallback&& cb) { codeCallback_ = cb; }
  void setPhraseCallback(PhraseCallback&& cb) { phraseCallback_ = cb; }

  void setHeaderCallback(HeaderCallback&& cb) { headerCallback_ = cb; }
  void setBodyCallback(BodyCallback&& cb) { bodyCallback_ = cb; }
#endif

  size_t parseExecute(const SockConnectionPtr&, const char*data, size_t len);
  bool parserError(){ return (mstate_<=MsgError); }
  MsgState getParseState() { return mstate_; }

  void reset();

  void eof() { eof_=true; }

  bool gotBody() { return (mstate_==MsgBody); }
  bool gotMessage() { return (mstate_==HeaderAll); }
  bool gotAll() { return (mstate_==MsgAll); }

  size_t nmessage() { return nmessage_; }
  size_t nbody() { return nbody_; }
  size_t nread() { return nread_; }

private:
  friend class TextMsgRequestCodec;
  friend class TextMsgResponseCodec;

  friend class TextMsgRequestCodec2;
  friend class TextMsgResponseCodec2;

  void setChunkedContent(bool t){ content_chunked_=t; }
  void setContentLengh(size_t len) {nbody_=len;}

private:
  bool requestMode(){ return (mmode_==MsgRequestMode); }
  bool responseMode(){ return (mmode_==MsgResponseMode); }

  bool processRequestFirstLine(const char* begin, size_t len);
  bool processResponseFirstLine(const char* begin, size_t len);
  bool processHeaderLine(const char* begin, size_t len);
  bool processBody(const SockConnectionPtr&, const char* begin, size_t len);
 
  bool processChunkHeaderLine(const char* begin, size_t len);
  bool parserChunkError(){ return (tstate_==ChunkError); }

  void resetBodyState(){ content_chunked_=false;tstate_=ChunkHeaderLine;chunked_size=0; nchunked_trailer_=0;}

private:
  ProtoCallback protoCallback_;
  MajorCallback majorCallback_;
  MinorCallback minorCallback_;

  //for request
  MethodCallback methodCallback_;
  PathCallback pathCallback_;
  QueryCallback queryCallback_;
  
  //for response
  CodeCallback codeCallback_;
  PhraseCallback phraseCallback_;

  HeaderCallback headerCallback_;
  BodyCallback bodyCallback_;

  TextLineParser lineParser_;

  MsgMode mmode_;

  MsgState mstate_;
  size_t nmessage_;// all message header parsed bytes by parser
  size_t nbody_;//all body parserd bytes by parser

  size_t nread_;//all parsed bytes by parser

  //for body private
  bool content_chunked_;
  size_t bindex_;
  size_t nchunked_trailer_;

  //for chunk body
  ChunkState tstate_;
  size_t chunked_size;

  bool eof_;
};

}
}

#endif//__FLAGWLD_NET_MSG_TEXTPARSER_H_
