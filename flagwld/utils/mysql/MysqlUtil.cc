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


#include <flagwld/utils/mysql/MysqlUtil.h>

#include <boost/scoped_array.hpp>

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <iostream>

namespace flagwld
{
namespace utils
{

static class __libmysql__
{
  public: __libmysql__() { mysql_library_init(0, NULL, NULL); }
  public: ~__libmysql__() { mysql_library_end(); }
} __libmysql__;

}
}

using namespace flagwld;
using namespace flagwld::utils;

#ifndef DEFAULT_QUERY_ARG_LEN
#define DEFAULT_QUERY_ARG_LEN 1024
#endif


/*============================================= Mysql =============================================*/

Mysql::Mysql():m_mysql_port(3306),
          m_mysql_handle(NULL)
{
}

Mysql::~Mysql()
{
  __reset();
}

string Mysql::error()
{
  assert(m_mysql_handle != NULL);

  return mysql_error(m_mysql_handle);
}

void Mysql::__reset()
{
  if (m_mysql_handle != NULL) {
    mysql_close(m_mysql_handle);
    m_mysql_handle = NULL;
  }
}

unsigned long Mysql::__escape(char *to, char const *from, unsigned long length)
{
  assert(m_mysql_handle != NULL);
  assert(mysql_errno(m_mysql_handle) == 0);

  return mysql_real_escape_string(m_mysql_handle, to, from, length);
}

bool Mysql::__query(string const &sql)
{
  assert(m_mysql_handle != NULL);
  assert(mysql_errno(m_mysql_handle) == 0);

  if (mysql_real_query(m_mysql_handle, sql.data(), sql.length()) != 0)
    return false;
  return true;
}

MYSQL_RES * Mysql::__store()
{
  assert(m_mysql_handle != NULL);
  assert(mysql_errno(m_mysql_handle) == 0);

  MYSQL_RES *res = mysql_store_result(m_mysql_handle);

  return res;
}

unsigned int Mysql::__fields()
{
  assert(m_mysql_handle != NULL);
  assert(mysql_errno(m_mysql_handle) == 0);

  return mysql_field_count(m_mysql_handle);
}

void Mysql::config(char const *server, char const *user, char const *password, unsigned short port)
{
  assert(server != NULL);
  assert(user != NULL);
  assert(password != NULL);
  assert(port > 0);

  __reset();
  m_mysql_server = server;
  m_mysql_user = user;
  m_mysql_password = password;
  m_mysql_port = port;
}

void Mysql::config(char const *server, char const *user, char const *password, char const *socket_file)
{
  assert(server != NULL);
  assert(user != NULL);
  assert(password != NULL);
  assert(socket_file != NULL);

  __reset();
  m_mysql_server = server;
  m_mysql_user = user;
  m_mysql_password = password;
  m_mysql_socket_file = socket_file;
}

bool Mysql::connect(char const *dbname, unsigned long client_flag)
{
  if (m_mysql_handle != NULL) {
    if (mysql_errno(m_mysql_handle) == 0)
      return true;
    __reset();
  }

  m_mysql_handle = mysql_init(NULL);
  if (NULL == m_mysql_handle){
    return false;
  }
     
  char const *mysql_socket_file =
     m_mysql_socket_file.empty() ? NULL : m_mysql_socket_file.c_str();

  if (NULL == mysql_real_connect(
			m_mysql_handle,
			m_mysql_server.c_str(),
			m_mysql_user.c_str(),
			m_mysql_password.c_str(),
			dbname,
			m_mysql_port,
			mysql_socket_file,
			client_flag))
    return false;

  return true;
}

void Mysql::close()
{
  __reset();
}

bool Mysql::execute(char const *format, ...)
{
  string query_string;
  va_list ap;
  va_start(ap, format);
  for (char const *ptr = format; *ptr != '\0'; ++ptr) {
    if (*ptr != '%') {
      query_string += *ptr;
      continue;
    }
    ++ptr;
    if (*ptr == '%') {
      query_string += *ptr;
      continue;
    }
    switch (*ptr) {
      case 's':
        query_string += va_arg(ap, char*);
        break;
      case 'q':
        {
          char const *argstr = va_arg(ap, char*);
	  size_t n = strlen(argstr);
          if (n * 2 < DEFAULT_QUERY_ARG_LEN) {
	    char buffer[DEFAULT_QUERY_ARG_LEN];
	    n = __escape(buffer, argstr, n);
	    query_string.append(buffer, n);
          } else {
            boost::scoped_array<char> buffer( new char[n * 2 + 1] );
            if (!buffer.get())
              return false;
	    n = __escape(buffer.get(), argstr, n);
	    query_string.append(buffer.get(), n);
          }
       }
       break;
     case 'd':
       {
         char buffer[32];
         int n = sprintf(buffer, "%d", va_arg(ap, int));
         query_string.append(buffer, n);
       }
       break;
     case 'u':
     {
       char buffer[32];
       int n = sprintf(buffer, "%u", va_arg(ap, unsigned int));
       query_string.append(buffer, n);
     }
     break;
     case 'f':
     {
       char buffer[32];
       int n = sprintf(buffer, "%.4f", va_arg(ap, double));
       query_string.append(buffer, n);
     }
     break;
     case 'l':
       ++ptr;
       switch (*ptr) {
         case 'l':
	   ++ptr;
	   switch (*ptr) {
             case 'u':
	       {
	         char buffer[32];
	         int n = sprintf(buffer, "%llu", va_arg(ap, unsigned long long));
	         query_string.append(buffer, n);
	       }
	       break;
	     case 'd':
	     default:
	       {
                 char buffer[32];
                 int n = sprintf(buffer, "%lld", va_arg(ap, long long));
                 query_string.append(buffer, n);
	       }
	       break;
          }
	  break;
        case 'u':
          {
            char buffer[32];
	    int n = sprintf(buffer, "%lu", va_arg(ap, unsigned long));
	    query_string.append(buffer, n);
          }
          break;
        case 'd':
        default:
          {
            char buffer[32];
            int n = sprintf(buffer, "%ld", va_arg(ap, long));
            query_string.append(buffer, n);
          }
          break;
      }
      break;
    default:
      query_string += *ptr;
      break;
    }
  }
  va_end(ap);

  std::cerr << query_string << std::endl;

  return __query(query_string);
}

unsigned long long Mysql::lastInsertId()
{
  assert(m_mysql_handle != NULL);
  assert(mysql_errno(m_mysql_handle) == 0);

  return mysql_insert_id(m_mysql_handle);
}

unsigned long long Mysql::updatedRows()
{
  assert(m_mysql_handle != NULL);
  assert(mysql_errno(m_mysql_handle) == 0);

  return mysql_affected_rows(m_mysql_handle);
}

ResultPtr Mysql::result()
{
  ResultPtr r;

  MYSQL_RES *mysql_result = __store();

  if (mysql_result){
    r.reset( new Result(mysql_result, __fields()) );
  }

  return r;
}

/*============================================= Result =============================================*/

Result::Result(): m_mysql_result(NULL),
    m_fields_count(0),
    m_rows_count(0)
{
}

Result::Result(MYSQL_RES *res, unsigned int filed): m_mysql_result(res),
    m_fields_count(filed),
    m_rows_count(mysql_num_rows(m_mysql_result))
{
}

Result::~Result()
{
  if (m_mysql_result != NULL) {
    mysql_free_result(m_mysql_result);
    m_mysql_result = NULL;
  }
}

bool Result::storeResult(const StoreFieldFunc& store)
{
  MYSQL_ROW row = mysql_fetch_row(m_mysql_result);

  assert(row != NULL);

  if (!row) return false;

  for (int i = 0; i < countFields(); i++) {
    if (!store(i, row[i])) return false;
  }

  return true;
}

int Result::countFields() const
{
  assert(m_mysql_result != NULL);

  return m_fields_count;
}

int64_t Result::countRows() const
{
  return m_rows_count;
}

