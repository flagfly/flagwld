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


#include <flagwld/utils/crypt/RSAUtil.h>

#include <flagwld/base/Logging.h>

#include <boost/bind.hpp>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace flagwld;
using namespace flagwld::utils;

unsigned char gkbuff[8092];

unsigned char gdbuff[8092];
int gdpos = 0;

bool datacb(unsigned char*d, size_t len)
{
  fprintf(stderr, "Got : %d\n", len);

  if ((gdpos+len)>sizeof(gdbuff))
  {
    fprintf(stderr, "Got X: %d\n", len);
    return false;
  }
  memcpy(gdbuff+gdpos, d, len);
  gdpos+=len;
  return true;
}
  
bool datacb2(unsigned char*d, size_t len)
{
  printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n[");
  for(int i=0;i<len;++i)
  {
    printf("%c", d[i]);
  }
  printf("]\n^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");

  return true;
}

int main()
{
  char *text = "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
  //char *text = "1234567890";

#if 0
/******************* pub key *******************/
{
  FILE* filep = fopen("key.pub", "rb");

  if(!filep)
  {
    LOG_ERROR << "File open error ";
    return -1;
  }

  struct stat st;
  int fd = fileno(filep);

  fstat(fd, &st);

  if (fread(gkbuff, 1,  st.st_size, filep) != st.st_size)
  {
    LOG_ERROR << "File read error ";
    return -1;
  }

  //fprintf(stderr, "%d - [%s]\n", st.st_size, gbuff);

  RSAPubEncoder pubenc((unsigned char*)gkbuff, st.st_size);
  pubenc.setDataCallback(datacb);

  pubenc.execute((unsigned char*)text, strlen(text));

  fclose(filep);
}

fprintf(stderr, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX gdpos=%d\n", gdpos);

/******************* pri key *******************/
{
  FILE* filep = fopen("key", "rb");

  if(!filep)
  {
    LOG_ERROR << "File open error ";
    return -1;
  }

  struct stat st;
  int fd = fileno(filep);

  fstat(fd, &st);

  if (fread(gkbuff, 1,  st.st_size, filep) != st.st_size)
  {
    LOG_ERROR << "File read error ";
    return -1;
  }

  //fprintf(stderr, "%d - [%s]\n", st.st_size, gbuff);

  RSAPriDecoder pridec(gkbuff, st.st_size);
  pridec.setDataCallback(datacb2);
  pridec.execute(gdbuff, gdpos);

  fclose(filep);
}

fprintf(stderr, "\nNext test:\n");

#endif

gdpos=0;

/******************* pri key *******************/
{
  FILE* filep = fopen("key", "rb");

  if(!filep)
  {
    LOG_ERROR << "File open error ";
    return -1;
  }

  struct stat st;
  int fd = fileno(filep);

  fstat(fd, &st);

  if (fread(gkbuff, 1,  st.st_size, filep) != st.st_size)
  {
    LOG_ERROR << "File read error ";
    return -1;
  }

  //fprintf(stderr, "%d - [%s]\n", st.st_size, gbuff);

  RSAPriEncoder prienc(gkbuff, st.st_size);
  prienc.setDataCallback(datacb);
  prienc.execute((unsigned char*)text, strlen(text));

  fclose(filep);
}

fprintf(stderr, "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX gdpos=%d\n", gdpos);

/******************* pub key *******************/
{   
  FILE* filep = fopen("key.pub", "rb");
  
  if(!filep)
  {
    LOG_ERROR << "File open error ";
    return -1;
  }

  struct stat st;
  int fd = fileno(filep);

  fstat(fd, &st);

  if (fread(gkbuff, 1,  st.st_size, filep) != st.st_size)
  {
    LOG_ERROR << "File read error ";
    return -1;
  }

  //fprintf(stderr, "%d - [%s]\n", st.st_size, gbuff);

  RSAPubDecoder pubdec((unsigned char*)gkbuff, st.st_size);
  pubdec.setDataCallback(datacb2);

  pubdec.execute(gdbuff, gdpos);

  fclose(filep);
}

  return 0;
}

