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


#ifndef FLAGWLD_UTILS_LIBZUTIL_H
#define FLAGWLD_UTILS_LIBZUTIL_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include <zlib.h>

namespace flagwld
{
namespace utils 
{

const int kZlibBuffer = 1024*4;

bool defaultZlibDataCallback(unsigned char*, size_t len);
typedef boost::function<bool (unsigned char*, size_t len)> ZlibDataCallback;

class ZEncoder: boost::noncopyable
{
public:
  ZEncoder();
  ~ZEncoder();

  inline void setDataCallback(const ZlibDataCallback& cb)
  { dataCallback_ = cb; }

  int execute(unsigned char*buff, size_t len, bool eof=true);

private:
  z_stream stream_;
  bool init_;
  ZlibDataCallback dataCallback_;
  unsigned char buffer_[kZlibBuffer];
  const int bufferSize_;
};

class ZDecoder: boost::noncopyable
{
public:
  ZDecoder();
  ~ZDecoder();

  inline void setDataCallback(const ZlibDataCallback& cb)
  { dataCallback_ = cb; }

  int execute(unsigned char*buff, size_t len);

private:
  z_stream stream_;
  bool init_;
  ZlibDataCallback dataCallback_;
  unsigned char buffer_[kZlibBuffer];
  const int bufferSize_;
};

}
}

#endif  // FLAGWLD_UTILS_LIBZUTIL_H
