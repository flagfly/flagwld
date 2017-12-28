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


#ifndef FLAGWLD_BASE_SCOPEGUARD_H
#define FLAGWLD_BASE_SCOPEGUARD_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

namespace flagwld
{

class ScopeGuard : boost::noncopyable
{
public:
  typedef boost::function<void ()> ScopeFunc;

  ScopeGuard(const ScopeFunc& func):func_(func),dismissed_(false)
  {
  }

#ifdef __GXX_EXPERIMENTAL_CXX0X__
  ScopeGuard(ScopeFunc&& func):func_(std::move(func)),dismissed_(false)
  {
  }
#endif

  ~ScopeGuard()
  {
    if (!dismissed_)
    {
      func_();
    }
  }

  inline void dismiss() const throw()
  {
    dismissed_ = true;
  }
private:
   ScopeFunc func_;
   mutable bool dismissed_;
};

}
#endif
