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


#ifndef FLAGWLD_UTILS_XXTEAUTIL_H
#define FLAGWLD_UTILS_XXTEAUTIL_H

#include <flagwld/base/Types.h>

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

namespace flagwld
{
namespace utils 
{

bool defaultXXTeaDataCallback(unsigned char*, size_t len);
typedef boost::function<bool (unsigned char*, size_t len)> XXTEADataCallback;

class XXTEAEncoder: boost::noncopyable
{
public:
  explicit XXTEAEncoder(const char*k);
  ~XXTEAEncoder();

  inline void setDataCallback(const XXTEADataCallback& cb)
  { dataCallback_ = cb; }

  int execute(unsigned char*buff, size_t len);

private:
  XXTEADataCallback dataCallback_;
  flagwld::string key_;

};

class XXTEADecoder: boost::noncopyable
{
public:
  explicit XXTEADecoder(const char*k);
  ~XXTEADecoder();

  inline void setDataCallback(const XXTEADataCallback& cb)
  { dataCallback_ = cb; }

  int execute(unsigned char*buff, size_t len);

private:
  XXTEADataCallback dataCallback_;
  flagwld::string key_;
};

}
}

#endif  // FLAGWLD_UTILS_XXTEAUTIL_H
