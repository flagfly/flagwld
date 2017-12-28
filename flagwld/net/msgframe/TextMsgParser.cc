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


#include <flagwld/net/msgframe/TextMsgParser.h>

#include <flagwld/base/Logging.h>
#include <flagwld/net/SockConnection.h>

#include <boost/bind.hpp>
#include <boost/any.hpp>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#undef __STDC_FORMAT_MACROS

using namespace flagwld;
using namespace flagwld::net;

/* Macros for character classes; depends on strict-mode  */
#define CR                  '\r'
#define LF                  '\n'
#define ISCR(c)             (c==CR)
#define ISLF(c)             (c==LF)
#define LOWER(c)            (unsigned char)(c | 0x20)
#define IS_ALPHA(c)         (LOWER(c) >= 'a' && LOWER(c) <= 'z')
#define IS_NUM(c)           ((c) >= '0' && (c) <= '9')
#define IS_ALPHANUM(c)      (IS_ALPHA(c) || IS_NUM(c))
#define IS_HEX(c)           (IS_NUM(c) || (LOWER(c) >= 'a' && LOWER(c) <= 'f'))

    
static const int8_t unhex[256] =
  {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
  ,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1                   
  ,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1                   
  , 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1                   
  ,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1                   
  ,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1                   
  ,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1
  ,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
  };


  typedef boost::function<void(const char*, const char*)> ProtoCallback;
  typedef boost::function<void(unsigned int)> MajorCallback;
  typedef boost::function<void(unsigned int)> MinorCallback;

  //for request
  typedef boost::function<void(const char*, const char*)> MethodCallback;
  typedef boost::function<void(const char*, const char*)> PathCallback;
  typedef boost::function<void(const char*, const char*)> QueryCallback;

  //for response
  typedef boost::function<void(int)> CodeCallback;
  typedef boost::function<void(const char*, const char*)> MessageCallback;


  typedef boost::function<void(const char*, const char*, const char*)> HeaderCallback;
  typedef boost::function<size_t(const char*, size_t)> BodyCallback;

void flagwld::net::defaultProtoCallback(const char*, const char*) { }
void flagwld::net::defaultMajorCallback(unsigned int) { }
void flagwld::net::defaultMinorCallback(unsigned int) { }
void flagwld::net::defaultMethodCallback(const char*, const char*) { }
void flagwld::net::defaultPathCallback(const char*, const char*) { }
void flagwld::net::defaultQueryCallback(const char*, const char*) { }
void flagwld::net::defaultCodeCallback(int) { }
void flagwld::net::defaultPhraseCallback(const char*, const char*) { }
void flagwld::net::defaultHeaderCallback(const char*, const char*, const char*) { }
size_t flagwld::net::defaultBodyCallback(const SockConnectionPtr& conn, const char*, size_t s) { return s;}

TextParser::TextParser(MsgMode m) 
          :protoCallback_(defaultProtoCallback),
           majorCallback_(defaultMajorCallback),
           minorCallback_(defaultMinorCallback),
           methodCallback_(defaultMethodCallback),
           pathCallback_(defaultPathCallback),
           queryCallback_(defaultQueryCallback),
           codeCallback_(defaultCodeCallback),
           phraseCallback_(defaultPhraseCallback),
           headerCallback_(defaultHeaderCallback),
           bodyCallback_(defaultBodyCallback),
           mmode_(m),
           eof_(false)
{
  assert( mmode_==MsgRequestMode || mmode_==MsgResponseMode );
  reset();
}

TextParser::~TextParser()
{
}
void TextParser::reset()
{
  mstate_ = FirstLine;
  nmessage_ = 0;
  nbody_ = 0;
  nread_ = 0;
  resetBodyState();

  lineParser_.resetLineState();
 
  eof_ = false;
}

/*
0: need parse more
>0: nparsed size
*/
#pragma GCC diagnostic ignored "-Wsign-compare"
size_t TextParser::parseExecute(const SockConnectionPtr& conn, const char*data, size_t len)
{
  assert (mstate_ != MsgAll);

  size_t nparsed=0, nline=0;

__reexecute_msg_byte:
  switch(mstate_) {
    case FirstLine: //receive line
      nline = lineParser_.readLine(data+nparsed, len-nparsed);
      if (lineParser_.parserLineError()) {
        LOG_DEBUG << "HeaderLine Error";
        mstate_ = MsgError;
        goto __parse_done;
      }
      if (nline == 0){
        goto __parse_done;
      }
      if (lineParser_.gotLine()) {
        nread_ += nline;
        if( nread_> MAX_MESSAGE_LEN ) {
          LOG_DEBUG << "FirstLine max exceed, Error";
          mstate_ = MsgError;
          nparsed += nline;
          goto __parse_done;
        }
        if (lineParser_.emptyLine()){
          LOG_DEBUG << "FirstLine empty, Error";
          mstate_=MsgError;
          nparsed += nline;
          goto __parse_done;
        }
        switch(mmode_) {
          case MsgRequestMode:
            if (!processRequestFirstLine( data+nparsed, lineParser_.nline()) ) {
              LOG_DEBUG << "Process Request FirstLine , Error";
              mstate_=MsgError;
              nparsed += nline;
              goto __parse_done;
            }
            break;
          case MsgResponseMode:
            if (!processResponseFirstLine( data+nparsed, lineParser_.nline()) ) {
              LOG_DEBUG << "Process Response FirstLine , Error";
              mstate_=MsgError;
              nparsed += nline;
              goto __parse_done;
            }
            break;
          default:
            assert(0);
            break;
        }
        mstate_=HeaderLine;
        lineParser_.resetLineState();
        nparsed += nline;
        if (nparsed<len) {
           goto __reexecute_msg_byte;
        }
      }

      break;

    case HeaderLine: //receive line
      nline = lineParser_.readLine(data+nparsed, len-nparsed);
      if (lineParser_.parserLineError()) {
        LOG_DEBUG << "HeaderLine read Error";
        mstate_ = MsgError;
        goto __parse_done;
      }
      if (nline == 0){
        goto __parse_done;
      }
      if (lineParser_.gotLine()) {
        nread_ += nline;
        if( nread_> MAX_MESSAGE_LEN ) {
          LOG_DEBUG << "HeaderLine max exceed, Error";
          mstate_ = MsgError;
          nparsed += nline;
          goto __parse_done;
        }
        if (lineParser_.emptyLine()){
          lineParser_.resetLineState();
          nparsed += nline;
          nmessage_=nread_; mstate_=HeaderAll;
          goto __parse_done;
        }
        if (!processHeaderLine(data+nparsed, lineParser_.nline())){
          LOG_DEBUG << "Process Request Headerline, Error";
          mstate_=MsgError;
          nparsed += nline;
          goto __parse_done;
        }
        lineParser_.resetLineState();
        nparsed += nline;
        if (nparsed<len) {
           goto __reexecute_msg_byte;
        }
      }

      break;

    case HeaderAll:
      bindex_ = 0;
      mstate_ = MsgBody;
    case MsgBody:
      //to do more with chunk or content-len
      if (!content_chunked_){
        if (nbody_==-1)
        {
          if (len>0)
          {
            if( !processBody(conn, data, len) ){
              LOG_DEBUG << "Process Msg Body , Error";
              mstate_=SysError;
              nparsed += len;
              nread_ += len;
              goto __parse_done;
            }
            nparsed += len;
            bindex_ += len;
            nread_ += len;
          }

          if (eof_)
          {
            nbody_ = bindex_;
            mstate_ = MsgAll;
            goto __reexecute_msg_byte;
          }
        }
        else
        {
          size_t l = nbody_-bindex_;
          size_t left = len-nparsed;
          l = l<left?l:left;
          if (l>0)
          {
            if( !processBody(conn, data+nparsed, l) ){
              LOG_DEBUG << "Process Msg Body , Error";
              mstate_=SysError;
              nparsed += l;
              goto __parse_done;
              nread_ += l;
            }
            nparsed += l;
            bindex_ += l;
            nread_ += l;
          }
          //fprintf(stderr, "Parsed body: %zd\n", l);
          if (bindex_==nbody_){
            mstate_ = MsgAll;
            goto __reexecute_msg_byte;
          }
        }
      } else { //recive chunk body
#if 0
        mstate_=MsgError; // not implement
        goto __parse_done;
#else // chunk implement
        switch(tstate_){
          case ChunkHeaderLine: 
            nline = lineParser_.readLine(data+nparsed, len-nparsed);
            if (lineParser_.parserLineError()) {
              LOG_DEBUG << "Msg Body Chunk read Headerline , Error";
              mstate_ = MsgError;
              goto __parse_done;
            }
            if (nline == 0){
              goto __parse_done;
            }
            if (lineParser_.gotLine()) {
              if (lineParser_.emptyLine()){
                LOG_DEBUG << "Msg Body Chunk empty Headerline , Error";
                mstate_ = MsgError;
                nread_ += nline;
                nparsed+=nline;
                goto __parse_done;
              }
              if (!processChunkHeaderLine(data+nparsed, lineParser_.nline())) {
                LOG_DEBUG << "Process Msg Body Chunk Headerline, Error";
                mstate_ = MsgError;
                nread_ += nline;
                nparsed+=nline;
                goto __parse_done;
              } 
              nread_ += nline;
              nparsed+=nline;
              lineParser_.resetLineState();
              if (chunked_size==0) { 
                 tstate_=ChunkTrailer;
              } else {
                 //fprintf(stderr, "Data: %"PRId64"\n",chunked_size);
                 bindex_ = 0;
                 tstate_=ChunkData;
              }
              if (nparsed<len) {
                goto __reexecute_msg_byte;
              }
            }
            break;
          case ChunkData:
            if (bindex_<chunked_size) { 
              size_t l = chunked_size-bindex_;
              size_t left = len-nparsed;
              l = l<left?l:left;
              if (l>0) {
                if( !processBody(conn, data+nparsed,l) ){
                  LOG_DEBUG << "Process Msg Body Chunk Body, Error";
                  mstate_=SysError;
                  nread_ += l;
                  nparsed+=l;
                  goto __parse_done;
                }
                nread_ += l;
                nparsed+=l;

                bindex_+=l;
                nbody_+=l;
              }
            } 
            if (bindex_==chunked_size) { 
              tstate_ = ChunkEndLine;
              if (nparsed<len) {
                goto __reexecute_msg_byte;
              }
            }
            break;
          case ChunkEndLine:
            nline = lineParser_.readLine(data+nparsed, len-nparsed);
            if (lineParser_.parserLineError()) {
              LOG_DEBUG << "Process Msg Body Chunk Endline, Error";
              mstate_ = MsgError;
              nread_ += nline;
              nparsed += nline;
              goto __parse_done;
            }   
            if (nline == 0){
              goto __parse_done;
            }
            if (lineParser_.gotLine()) {
              if (!lineParser_.emptyLine()){
                LOG_DEBUG << "Process Msg Body Chunk Endline Empty, Error";
                mstate_ = MsgError;
                nread_ += nline;
                nparsed += nline;
                goto __parse_done;
              }
              lineParser_.resetLineState();
              chunked_size = 0;
              tstate_ = ChunkHeaderLine;

              nread_ += nline;
              nparsed += nline;

              if (nparsed<len) {
                goto __reexecute_msg_byte;
              }
            }
            break;
          case ChunkTrailer:
            nline = lineParser_.readLine(data+nparsed, len-nparsed);
            if (lineParser_.parserLineError()) {
              LOG_DEBUG << "Process Msg Body Chunk Trailer, Error";
              mstate_ = MsgError;
              goto __parse_done;
            }
            if (nline == 0){
              goto __parse_done;
            }
            if (lineParser_.gotLine()) {
              nchunked_trailer_ += nline;
              if (nchunked_trailer_>MAX_MESSAGE_LEN)
              {
                LOG_DEBUG << "Process Msg Body Chunk Trailer max exceed, Error";
                mstate_ = MsgError;
                nread_ += nline;
                nparsed += nline;
                goto __parse_done;
              }
              nread_ += nline;
              nparsed += nline;
              if (lineParser_.emptyLine()){
                mstate_ = MsgAll;
                goto __reexecute_msg_byte;
              }
              //ignore trailer header  

              if (nparsed<len) {
                goto __reexecute_msg_byte;
              }
              
            }
            break;
          default:
            LOG_DEBUG << "Process Msg Body Chunk default, Error";
            mstate_ = MsgError;
            goto __parse_done;
            break;
        }
#endif
      }
      break;
    case MsgAll:
      LOG_TRACE << "Msg Received: Total:" << nread_ << " Header:" << nmessage_ << " Body:" << nbody_;
      goto __parse_done;
      break;
    default:
      LOG_DEBUG << "Process Msg default, Error " << mstate_;
      mstate_ = MsgError;
      goto __parse_done;
      break;
  }

__parse_done:
  //fprintf(stderr, ">>>> %s:%s  >> %d >>>>>>  nparsed=%zd len=%zd\n", __FILE__, __FUNCTION__, __LINE__, nparsed, len);
  return nparsed;
}
#pragma GCC diagnostic error "-Wsign-compare"

bool TextParser::processRequestFirstLine(const char* begin, size_t len)
{
  //size_t i=0; fprintf(stderr, "%s - FirstLine[%zd]: [", __func__, len); for(i=0; i<len; ++i ) fprintf(stderr, "%c", begin[i]); fprintf(stderr, "]\n");
  bool succeed = false;
  const char* start = begin;
  const char* end= begin+len;
  const char* space = std::find(start, end, ' ');
  if (space != end )
  {
    methodCallback_(start, space);

    start = space+1;
    space = std::find(start, end, ' ');
    if (space != end)
    {
      const char* question = std::find(start, space, '?');
      if (question != space)
      {
        pathCallback_(start, question);
        queryCallback_(question+1, space);
      }
      else
      {
        pathCallback_(start, space);
      }
      start = space+1;
      const char* slash = std::find(start, end, '/');
      if (slash != end)
      {
        protoCallback_(start, slash);
        start = slash+1;
        unsigned int v=0; 
        const char* dot = std::find(start, end, '.');
        if (dot != end )
        { 
          while (start!=dot)
          { 
            if (IS_NUM(*start))
            { 
              v = v*10 + (*start-'0');
            }
            else
            { 
              break;
            }
            start++;
          }
          if (start==dot)
          { 
            majorCallback_(v);
            start=dot+1;
            v=0;
            while (start!=end)
            { 
              if (IS_NUM(*start))
              { 
                v = v*10 + (*start-'0');
              }
              else
              { 
                break;
              }
              start++;
            }
            if(start==end)
            { 
              minorCallback_(v);
              succeed=true;
            }
          }
        }
      }
    }
  }
  return succeed;
}

bool TextParser::processResponseFirstLine(const char* begin, size_t len)
{
  //size_t i=0; fprintf(stderr, "%s - FirstLine[%zd]: [", __func__, len); for(i=0; i<len; ++i ) fprintf(stderr, "%c", begin[i]); fprintf(stderr, "]\n");
  bool succeed = false;
               
  const char* start = begin;
  const char* end = begin+len;
  const char* slash = std::find(start, end, '/');
  if (slash != end)
  { 
    protoCallback_(start, slash);
    start = slash+1;
    unsigned int v=0;
    const char* dot = std::find(start, end, '.');
    if (dot != end )
    {
      while (start!=dot)
      {
        if (IS_NUM(*start))
        {
          v = v*10 + (*start-'0');
        }
        else
        {
          break;
        }
        start++;
      }
      if (start == dot)
      {
        majorCallback_(v);
        start=dot+1;
        v=0;
        const char* space = std::find(start, end, ' ');
        if (space != end)
        {
          while (start != space)
          {
            if (IS_NUM(*start))
            {
              v = v*10 + (*start-'0');
            }
            else
            {
              break;
            }
            start++;
          }
          if(start==space)
          {
            minorCallback_(v);
            start = space+1;
            v=0;
            space = std::find(start, end, ' ');
            if (space != end)
            {
              while (start != space)
              {
                if (IS_NUM(*start))
                {
                  v = v*10 + (*start-'0');
                }
                else
                {
                  break;
                }
                start++;
               }
               if(start==space)
               {
                 codeCallback_(v);
                 start = space+1;
                 phraseCallback_(start, end);
                 succeed = true;
               }
             }
          }
        }
      }
    }
  }

  return succeed;
}

bool TextParser::processHeaderLine(const char* begin, size_t len)
{
  bool succeed = true;
  //size_t i=0; fprintf(stderr, "%s - HeaderLine[%zd]: [", __func__, len); for(i=0; i<len; ++i ) fprintf(stderr, "%c", begin[i]); fprintf(stderr, "]\n");

  const char*end = begin+len;
  const char* colon = std::find(begin, end, ':');
  if (colon != end)
  {
    headerCallback_(begin, colon, end);
  }
  else
  {
    succeed = false;
  }

  return succeed;
}

bool TextParser::processBody(const SockConnectionPtr& conn, const char* begin, size_t len)
{
  //size_t i=0; fprintf(stderr, "%s - Body[%zd]: [", __func__, len); for(i=0; i<len; ++i ) fprintf(stderr, "%c", begin[i]); fprintf(stderr, "]\n");
  bool succeed = true;
  
  if( bodyCallback_(conn, begin, len) != len ){
    succeed = false;
  }
  
  //fprintf(stderr, ">>>> %s:%s  >> %d >>>>>> success=%d\n", __FILE__, __FUNCTION__, __LINE__, succeed);
  return succeed;
}
#pragma GCC diagnostic ignored "-Wold-style-cast"
bool TextParser::processChunkHeaderLine(const char* begin, size_t len)
{
  //size_t i=0; fprintf(stderr, "%s - processChunkHeaderLine[%zd]: [", __func__, len); for(i=0; i<len; ++i ) fprintf(stderr, "%c", begin[i]); fprintf(stderr, "]\n");
  bool succeed = false;

  const char* start = begin;
  const char* end= begin+len;
  const char* semicolon = std::find(start, end, ';');
  
  chunked_size = 0;

  if (semicolon != end) {
    end = semicolon;
  }

  while (start != end) {
    if (!IS_HEX(*start)) {
      goto __end;
    }
    chunked_size = chunked_size*16 + unhex[(unsigned char)(*start)];
    start++;
  }
  succeed = true;
__end:
  //fprintf(stderr, ">>>> %s:%s  >> %d >>>>>> succeed=%d chunksize=%ld\n", __FILE__, __FUNCTION__, __LINE__, succeed, chunked_size);
  return succeed;
}
#pragma GCC diagnostic error "-Wold-style-cast"
