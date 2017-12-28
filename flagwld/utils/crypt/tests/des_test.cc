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


#include <flagwld/utils/crypt/DESUtil.h>

#include <flagwld/base/Logging.h>

#include <boost/bind.hpp>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace flagwld;
using namespace flagwld::utils;

unsigned char gdbuff[8092] = {0};
int gdpos = 0;

bool datacb(unsigned char*d, size_t len)
{
  fprintf(stderr, "EN_Got : %d\n", len);

  fprintf(stderr, "[");
  for(int i=0;i<len;++i) { fprintf(stderr, "%02x ", d[i]); }
  fprintf(stderr, "]\n");

  if ((gdpos+len)>sizeof(gdbuff))
  {
    fprintf(stderr, "err EN_Got X: %d\n", len);
    return false;
  }
  memcpy(gdbuff+gdpos, d, len);
  gdpos+=len;
  return true;
}
  
bool datacb2(unsigned char*d, size_t len)
{
  fprintf(stderr, "DE_Got : %d\n", len);
  fprintf(stderr, "[");
  for(int i=0;i<len;++i) { fprintf(stderr, "%c ", d[i]); }
  fprintf(stderr, "]\n");

  return true;
}

int main()
{
  char *text = "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
  //char *text = "12345678";
  //char *text = "1234";

//while(true)
{
gdpos=0;
  
fprintf(stderr, "[");
for(int i=0;i<strlen(text);++i) { fprintf(stderr, "%c ", text[i]); }
fprintf(stderr, "]\n");

{
  DES3Encoder enc("x1", "x2", "x3", "ivec");
  enc.setDataCallback(datacb);
  enc.execute((unsigned char*)text, strlen(text));
}

fprintf(stderr, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX gdpos=%d\n", gdpos);

{   
  DES3Decoder dec("x1", "x2", "x3", "ivec");
  dec.setDataCallback(datacb2);
  dec.execute(gdbuff, gdpos);
}
}

  return 0;
}

