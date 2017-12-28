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


#ifndef FLAGWLD_UTILS_MD5_H
#define FLAGWLD_UTILS_MD5_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include <openssl/md5.h>

namespace flagwld
{
namespace utils 
{

bool defaultMd5DataCallback(unsigned char*, size_t len);
typedef boost::function<bool (unsigned char*, size_t len)> MD5DataCallback;

const size_t kMd5Buffer = MD5_DIGEST_LENGTH;

class MD5Encoder: boost::noncopyable
{
public:
  explicit MD5Encoder();
  ~MD5Encoder();

  inline void setDataCallback(const MD5DataCallback& cb)
  { dataCallback_ = cb; }

  int execute(unsigned char*buff, size_t len, bool eof=true);

private:
  MD5DataCallback dataCallback_;
  unsigned char buffer_[kMd5Buffer];
  const size_t bufferSize_;
   
  MD5_CTX md5_;  
};

}
}

#endif  // FLAGWLD_UTILS_MD5_H
