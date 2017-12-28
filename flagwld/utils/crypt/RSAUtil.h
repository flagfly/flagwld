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


#ifndef FLAGWLD_UTILS_RSAUTIL_H
#define FLAGWLD_UTILS_RSAUTIL_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include <openssl/rsa.h>

namespace flagwld
{
namespace utils 
{

const int kRsaBuffer = 1024;

bool defaultRSADataCallback(unsigned char*, size_t len);
typedef boost::function<bool (unsigned char*, size_t len)> RSADataCallback;

class RSAPubEncoder: boost::noncopyable
{
public:
  explicit RSAPubEncoder(unsigned char*, int);
  ~RSAPubEncoder();

  inline void setDataCallback(const RSADataCallback& cb)
  { dataCallback_ = cb; }

  int execute(unsigned char*buff, size_t len);

private:
  RSA *rsa_;
  RSADataCallback dataCallback_;
  unsigned char buffer_[kRsaBuffer];
  const int bufferSize_;
  int rsaSize_;
};

class RSAPriDecoder: boost::noncopyable
{
public:
  explicit RSAPriDecoder(unsigned char*, int);
  ~RSAPriDecoder();

  inline void setDataCallback(const RSADataCallback& cb)
  { dataCallback_ = cb; }

  int execute(unsigned char*buff, size_t len);

private:
  RSA *rsa_;
  RSADataCallback dataCallback_;
  unsigned char buffer_[kRsaBuffer];
  const int bufferSize_;
  int rsaSize_;
};

class RSAPriEncoder: boost::noncopyable
{
public:
  explicit RSAPriEncoder(unsigned char*, int);
  ~RSAPriEncoder();

  inline void setDataCallback(const RSADataCallback& cb)
  { dataCallback_ = cb; }

  int execute(unsigned char*buff, size_t len);

private:
  RSA *rsa_;
  RSADataCallback dataCallback_;
  unsigned char buffer_[kRsaBuffer];
  const int bufferSize_;
  int rsaSize_;
};

class RSAPubDecoder: boost::noncopyable
{
public:
  explicit RSAPubDecoder(unsigned char*, int);
  ~RSAPubDecoder();

  inline void setDataCallback(const RSADataCallback& cb)
  { dataCallback_ = cb; }

  int execute(unsigned char*buff, size_t len);

private:
  RSA *rsa_;
  RSADataCallback dataCallback_;
  unsigned char buffer_[kRsaBuffer];
  const int bufferSize_;
  int rsaSize_;
};

}
}

#endif  // FLAGWLD_UTILS_RSAUTIL_H
