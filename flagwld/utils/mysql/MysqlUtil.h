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


#ifndef FLAGWLD_UTILS_MYSQLUTIL_H
#define FLAGWLD_UTILS_MYSQLUTIL_H

#include <flagwld/base/Types.h>
#include <flagwld/base/StringPiece.h>

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

#include <mysql/mysql.h>

namespace flagwld
{
namespace utils
{

class Result;
class Mysql;
typedef boost::shared_ptr<Result> ResultPtr;
typedef boost::shared_ptr<Mysql> MysqlPtr;

class Mysql: boost::noncopyable
{
public:
  Mysql();
  ~Mysql();

  void close();

  bool connect(char const *dbname, unsigned long client_flag=0);

  string error();

  void config(char const *server,
          char const *user,
          char const *password,
          unsigned short port=3306);
  void config(char const *server,
          char const *user,
          char const *password,
          char const *socket_file);

  bool execute(char const *format, ...);
  unsigned long long lastInsertId();
  unsigned long long updatedRows();

  ResultPtr result();

private:
  void __reset();
  char const * __error();
  unsigned long __escape(char *to, char const *from, unsigned long length);
  bool __query(string const &sql);
  MYSQL_RES * __store();
  unsigned int __fields();

private:
  string m_mysql_server;
  string m_mysql_user;
  string m_mysql_password;
  string m_mysql_socket_file;
  unsigned short m_mysql_port;
  MYSQL *m_mysql_handle;
};

class Result : boost::noncopyable
{
public:
  typedef boost::function<bool(int, char const*)> StoreFieldFunc;
  friend class Mysql;

  ~Result();

  bool storeResult(const StoreFieldFunc& store);
  int countFields() const;
  int64_t countRows() const;

private:
  explicit Result();
  explicit Result(MYSQL_RES*res, unsigned int);

  MYSQL_RES *m_mysql_result;
  unsigned int m_fields_count;
  unsigned long long m_rows_count;
};

}
}

#endif  // FLAGWLD_UTILS_MYSQLUTIL_H
