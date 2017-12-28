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


#include <flagwld/net/msgframe/TextLineParser.h>

#include <flagwld/base/Logging.h>

using namespace flagwld;
using namespace flagwld::net;

/* Macros for character classes; depends on strict-mode  */
#define CR                  '\r'
#define LF                  '\n'
#define ISCR(c)             (c==CR)
#define ISLF(c)             (c==LF)
    
TextLineParser::TextLineParser():
                 lstate_(LineCR),
                 lindex_(0),
                 nline_(0),
                 compatible_(true)
{
}

TextLineParser::~TextLineParser()
{
}

/*
0: need parse more
>0: got line, return line length
*/
size_t TextLineParser::readLine(const char*data, size_t len)
{
  assert (lstate_ == LineCR || lstate_ == LineLF); 

  size_t nparsed=0;

__reexecute_line_byte:
  switch (lstate_)
  {
    case LineCR:
      while(lindex_<len)
      {
        if (ISCR(data[lindex_++])){ // \r\n
          lstate_ = LineLF;
          goto __reexecute_line_byte;
        } else if (compatible_ && ISLF(data[lindex_-1])){ // \n
          nparsed=lindex_;
          lstate_ = LineEnd;
          goto __parse_line_done;
        }
        if (++nline_>MAX_LINE_LEN)
        {
          LOG_ERROR ;
          nparsed=lindex_;
          lstate_ = LineError;
          goto __parse_line_done;
        }
      }
      
      break;

    case LineLF:
      if (lindex_<len) {
        if (ISLF(data[lindex_++])){
          nparsed=lindex_;
          lstate_ = LineEnd;
          goto __parse_line_done;
        } else {
          LOG_ERROR ;
          nparsed=lindex_;
          lstate_ = LineError;
          goto __parse_line_done;
        }
      }
      break;

    case LineEnd:
    default:
      LOG_ERROR ;
      lstate_ = LineError;
      goto __parse_line_done;
      break;
  }

__parse_line_done:
  //fprintf(stderr, ">>>> %s:%s  >> %d >>>>>> input=%zd lindex=%zd nline=%zd nparsed=%zd\n", __FILE__, __FUNCTION__, __LINE__, len, lindex_, nline_, nparsed);
  return nparsed;
}
