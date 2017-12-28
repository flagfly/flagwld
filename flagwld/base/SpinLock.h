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


#ifndef FLAGWLD_BASE_SPINLOCk_H
#define FLAGWLD_BASE_SPINLOCK_H

#include <flagwld/base/CurrentThread.h>
#include <boost/noncopyable.hpp>
#include <assert.h>
#include <pthread.h>

#ifdef CHECK_PTHREAD_RETURN_VALUE

#ifdef NDEBUG
__BEGIN_DECLS
extern void __assert_perror_fail (int errnum,
                                  const char *file,
                                  unsigned int line,
                                  const char *function)
    __THROW __attribute__ ((__noreturn__));
__END_DECLS
#endif

#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       if (__builtin_expect(errnum != 0, 0))    \
                         __assert_perror_fail (errnum, __FILE__, __LINE__, __func__);})

#else  // CHECK_PTHREAD_RETURN_VALUE

#define MCHECK(ret) ({ __typeof__ (ret) errnum = (ret);         \
                       assert(errnum == 0); (void) errnum;})

#endif // CHECK_PTHREAD_RETURN_VALUE

namespace flagwld
{

class SpinLock : boost::noncopyable
{ 
 public:
  SpinLock()
    : holder_(0)
  {
    MCHECK(pthread_spin_init(&spinlock_, PTHREAD_PROCESS_PRIVATE));
  }

  ~SpinLock()
  {
    assert(holder_ == 0);
    MCHECK(pthread_spin_destroy(&spinlock_));
  }  

  // must be called when locked, i.e. for assertion
  bool isLockedByThisThread() const
  {
    return holder_ == CurrentThread::tid();
  } 
  
  void assertLocked() const
  {
    assert(isLockedByThisThread());
  }
  
  // internal usage

  void lock()
  {
    MCHECK(pthread_spin_lock(&spinlock_));
    assignHolder();
  }

  void unlock()
  {
    unassignHolder();
    MCHECK(pthread_spin_unlock(&spinlock_));
  }

  pthread_spinlock_t* getPthreadSpinLock() /* non-const */
  {
    return &spinlock_;
  }

 private:
  class UnassignGuard : boost::noncopyable
  {
   public:
    UnassignGuard(SpinLock& owner)
      : owner_(owner)
    {
      owner_.unassignHolder();
    }

    ~UnassignGuard()
    {
      owner_.assignHolder();
    }

   private:
    SpinLock& owner_;
  };

  void unassignHolder()
  {
    holder_ = 0;
  }

  void assignHolder()
  {
    holder_ = CurrentThread::tid();
  }

  pthread_spinlock_t spinlock_;
  pid_t holder_;
};

class SpinLockGuard : boost::noncopyable
{
 public:
  explicit SpinLockGuard(SpinLock& spinlock)
    : spinlock_(spinlock)
  {
    spinlock_.lock();
  }

  ~SpinLockGuard()
  {
    spinlock_.unlock();
  }

 private:
  SpinLock& spinlock_;
};
}

// Prevent misuse like:
// SpinLockGuard(mutex_);
// A tempory object doesn't hold the lock for long!
#define SpinLockGuard(x) error "Missing guard object name"

#endif  // FLAGWLD_BASE_SPINLOCK_H
