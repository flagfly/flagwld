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


#include <flagwld/utils/zlib/ZUtil.h>

#include <flagwld/base/Logging.h>

using namespace flagwld;
using namespace flagwld::utils;

bool flagwld::utils::defaultZlibDataCallback(unsigned char*, size_t len)
{
  LOG_TRACE << "Zlib process ouput " << len;
  return true;
}

/***************************************** encoder *********************************/
#pragma GCC diagnostic ignored "-Wold-style-cast"
ZEncoder::ZEncoder()
           :init_(false),
            dataCallback_(defaultZlibDataCallback),
            bufferSize_(sizeof(buffer_))
       
{
  stream_.zalloc = Z_NULL;
  stream_.zfree = Z_NULL;
  stream_.opaque = Z_NULL;
  
  int err = deflateInit(&stream_, Z_DEFAULT_COMPRESSION);
  if (err==Z_OK)
  {
    init_=true;
  }
  else
  {
    LOG_ERROR << "Zlib deflateInit Error: " << err;
  }
}
#pragma GCC diagnostic error "-Wold-style-cast"

ZEncoder::~ZEncoder()
{
  if (init_)
  {
    int err = deflateEnd(&stream_);
    if (err!=Z_OK)
    {
      LOG_ERROR << "Zlib deflateEnd Error: " << err;
    }
  }
  init_=false;
}

int ZEncoder::execute(unsigned char*buff, size_t len, bool eof)
{
  assert (init_);

  if (!init_)
  {
    return -1;
  }

  int flush = eof ? Z_FINISH : Z_NO_FLUSH;

  stream_.next_in = buff;
  stream_.avail_in = static_cast<uInt>(len);

  do
  {
    stream_.next_out = buffer_;
    stream_.avail_out = bufferSize_;
     
    int err = deflate(&stream_, flush);

    assert (err!=Z_STREAM_ERROR);

    if (!(err==Z_OK || err==Z_STREAM_END))
    {
      LOG_ERROR << "Zlib deflate Error: " << err;
      return -1;
    }
         
    size_t have = bufferSize_ - stream_.avail_out;

    if (have>0 && !dataCallback_(buffer_, have))
    {
      return -1;
    }
  }
  while(stream_.avail_out == 0);

  assert(stream_.avail_in == 0);

  LOG_DEBUG <<"Compress:  " << (len-stream_.avail_in);

  return static_cast<int>(len-stream_.avail_in);
}


/***************************************** decoder *********************************/
#pragma GCC diagnostic ignored "-Wold-style-cast"
ZDecoder::ZDecoder()
           :init_(false),
            dataCallback_(defaultZlibDataCallback),
            bufferSize_(sizeof(buffer_))
{
  stream_.zalloc = Z_NULL;
  stream_.zfree = Z_NULL;
  stream_.opaque = Z_NULL;
  
  int err = inflateInit(&stream_);
  if (err==Z_OK)
  {
    init_=true;
  }
  else
  {
    LOG_ERROR << "Zlib inflateInit Error: " << err;
  }
}
#pragma GCC diagnostic error "-Wold-style-cast"

ZDecoder::~ZDecoder()
{
  if (init_)
  {
    int err = inflateEnd(&stream_);
    if (err!=Z_OK)
    {
      LOG_ERROR << "Zlib inflateEnd Error\n";
    }
  }
  init_=false;
}

int ZDecoder::execute(unsigned char*buff, size_t len)
{
  assert (init_);

  if (!init_)
  {
    return -1;
  }

  stream_.next_in = buff;
  stream_.avail_in = static_cast<uInt>(len);

  do
  {
    stream_.next_out = buffer_;
    stream_.avail_out = bufferSize_;
     
    int err = inflate(&stream_, Z_NO_FLUSH);

    assert(err != Z_STREAM_ERROR);

    if (!(err==Z_OK || err==Z_STREAM_END))
    {
      LOG_ERROR << "Zlib deflate Error: " << err;
      return -1;
    }
    size_t have = bufferSize_ - stream_.avail_out;

    if (have>0 && !dataCallback_(buffer_, have))
    {
      return -1;
    }
  }
  while(stream_.avail_out == 0);

  LOG_DEBUG << "uncompress: " << (len-stream_.avail_in); 

  return static_cast<uInt>(len-stream_.avail_in);
}

