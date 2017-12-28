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


#include <flagwld/utils/crypt/XXTEAUtil.h>
#include <flagwld/utils/base62.h>

#include <flagwld/base/Logging.h>
#include <flagwld/net/Buffer.h>

#include <boost/bind.hpp>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace flagwld;
using namespace flagwld::net;
using namespace flagwld::utils;

Buffer enbuff;
Buffer debuff;

#pragma GCC diagnostic ignored "-Wold-style-cast"

bool Save(Buffer* buffer, unsigned char* data, size_t len)
{
  buffer->append(data, len);

  return true;
}

int main()
{

  const char* name = "abcdefghijklmn";

  XXTEAEncoder en("abcde");
  en.setDataCallback(boost::bind(&Save, &enbuff, _1, _2));
  int len = en.execute((unsigned char*)name, strlen(name));
  printf("Len: %d  %ld\n", len, enbuff.readableBytes());
  
  char x[1024] = {0};
  
  size_t xlen = base62_encode((const unsigned char*)enbuff.peek(), enbuff.readableBytes(), x, sizeof(x)-1);

  printf("%d   %ld %s\n", base62_encoded_length(enbuff.readableBytes()), xlen, x);
  

  XXTEADecoder de("abcde");
  de.setDataCallback(boost::bind(&Save, &debuff, _1, _2));

  len = de.execute((unsigned char*)enbuff.peek(), len);
  printf("Len: %d  %ld  -  %s\n", len, debuff.readableBytes(), debuff.peek());


  return 0;
}

