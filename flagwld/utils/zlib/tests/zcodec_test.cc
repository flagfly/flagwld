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

#include <boost/bind.hpp>

#include <stdio.h>

using namespace flagwld;
using namespace flagwld::utils;

FILE* gcompress_fp=NULL;
bool savecompress(unsigned char* data, size_t len)
{
  assert (gcompress_fp); 

  fprintf(stderr, "output : %d\n", len);
  return (fwrite(data, 1, len, gcompress_fp)==len);
}

FILE* guncompress_fp=NULL;
bool saveuncompress(unsigned char* data, size_t len)
{
  assert (gcompress_fp);

  for(int i=0;i<len; ++i)
    fprintf(stderr, "%c", data[i]);
  fprintf(stderr, "\n");
  return (fwrite(data, 1, len, guncompress_fp)==len);
}

int main()
{
  {
    ZEncoder encoder;
    encoder.setDataCallback(savecompress);
    gcompress_fp = fopen("z.c", "w+");
  
    if(!gcompress_fp)
    {
      fprintf(stderr, "open file error.\n");
      return -1;
    }
  
#if 0
    const char *buf = "1234567890abcdefghijklmnopqrstuvwxyz";
    size_t len = strlen(buf);
  
    for(int i=0;i<len; ++i)
      fprintf(stderr, "%c", buf[i]);
    fprintf(stderr, "\n");
  
    for(int i=0; i<100; i++)
    {
      bool eof=false;
      if (i==99) eof=true;
      if (encoder.compress((unsigned char*)buf, len, eof)<0)
      {
        fprintf(stderr, "compress error.\n");
        return -1;
      }
    }
#else 
    unsigned char buf[4096];
    FILE* fp = fopen("t.o", "r");
    int len = fread(buf, 1, sizeof(buf), fp); 
    fclose(fp);
    fprintf(stderr, "Read %d\n", len);

    if (encoder.execute((unsigned char*)buf, len, true)<0)
    {
      fprintf(stderr, "compress error.\n");
      return -1;
    }
#endif
    fclose(gcompress_fp);
    fprintf(stderr, "compress ok.\n");
  }

  {
    ZDecoder decoder;
    decoder.setDataCallback(saveuncompress);
    guncompress_fp = fopen("z.d", "w+");
  
    if(!guncompress_fp)
    {
      fprintf(stderr, "open file error.\n");
      return -1;
    }
  
    gcompress_fp = fopen("z.c", "r");
  
    if(!gcompress_fp)
    {
      fprintf(stderr, "open file error.\n");
      return -1;
    }
  
    unsigned char buf2[1024];
  
    while(!feof(gcompress_fp))
    {
      int l = fread(buf2, 1, sizeof(buf2), gcompress_fp);
      fprintf(stderr, "Read z data: %d\n", l );
  
      if (decoder.execute((unsigned char*)buf2, l)<0)
      {
        fprintf(stderr, "uncompress error.\n");
        return -1;
      }
    }
  
    fclose(guncompress_fp);
    fclose(gcompress_fp);
  
    fprintf(stderr, "uncompress ok.\n");
  }

  return 0; 
}

