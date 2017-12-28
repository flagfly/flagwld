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


bool makefiledirs(string const &path) 
{ 
  size_t index = path.find_last_of("\\/");
  if (index == 0)
  {
    return true;
  }
  string const &_path = path.substr(0, index);
  if (access(_path.c_str(), F_OK) == 0)
  {
    return true;
  }
  if (index != string::npos)
  {
    if (!makefiledirs(_path))
    {
      return false;
    }
  }
  if (mkdir(_path.c_str(), 0755) != 0)
  {
    return false;
  }
  return true;
}
