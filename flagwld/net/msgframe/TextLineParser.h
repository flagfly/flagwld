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


#ifndef __FLAGWLD_NET_MSG_TEXTLINEPARSER_H_
#define __FLAGWLD_NET_MSG_TEXTLINEPARSER_H_

#include <flagwld/net/msgframe/Message.h>

#include <boost/noncopyable.hpp>

#include <sys/types.h>

namespace flagwld
{
namespace net
{

class TextLineParser : boost::noncopyable
{
public:
  enum LineState
  {
    LineError=-1,
    LineCR,
    LineLF,
    LineEnd,
  };
  
public:
  TextLineParser();
  ~TextLineParser();

  size_t readLine(const char*data, size_t len);

  inline bool gotLine(){ return (lstate_==LineEnd); }
  inline bool emptyLine(){ return (nline_==0); }
  inline size_t nread(){ return lindex_; }
  inline size_t nline(){ return nline_; } //without \n or \r\n

  inline bool parserLineError() { return (lstate_==LineError); }
  inline void resetLineState(){ lstate_=LineCR; lindex_=0; nline_=0; }

  inline void setCompatible(bool f) { compatible_=f; }

private:
  LineState lstate_;
  size_t lindex_;
  size_t nline_;

  bool compatible_;
};

}
}

#endif//__FLAGWLD_NET_MSG_TEXTLINEPARSER_H_
