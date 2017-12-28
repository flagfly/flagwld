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


#ifndef FLAGWLD_UTILS_DESUTIL_H
#define FLAGWLD_UTILS_DESUTIL_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include <openssl/des.h>

namespace flagwld
{
namespace utils 
{

const int kDesBuffer = 64;

bool defaultDesDataCallback(unsigned char*, size_t len);
typedef boost::function<bool (unsigned char*, size_t len)> DESDataCallback;

class DES3Encoder: boost::noncopyable
{
public:
  explicit DES3Encoder(const char*k1,
                       const char*k2,
                       const char*k3,
                       const char*ivec);
  ~DES3Encoder();

  inline void setDataCallback(const DESDataCallback& cb)
  { dataCallback_ = cb; }

  int execute(unsigned char*buff, size_t len);

private:
  DESDataCallback dataCallback_;
  unsigned char buffer_[kDesBuffer];
  const int bufferSize_;
  
  DES_key_schedule ks1_;
  DES_key_schedule ks2_;
  DES_key_schedule ks3_;

  DES_cblock ivec_;
};

class DES3Decoder: boost::noncopyable
{
public:
  explicit DES3Decoder(const char*k1,
                       const char*k2,
                       const char*k3,
                       const char*ivec);
  ~DES3Decoder();

  inline void setDataCallback(const DESDataCallback& cb)
  { dataCallback_ = cb; }

  int execute(unsigned char*buff, size_t len);

private:
  DESDataCallback dataCallback_;
  unsigned char buffer_[kDesBuffer];
  const int bufferSize_;

  DES_key_schedule ks1_;
  DES_key_schedule ks2_;
  DES_key_schedule ks3_;

  DES_cblock ivec_;
};

}
}

#endif  // FLAGWLD_UTILS_DESUTIL_H
