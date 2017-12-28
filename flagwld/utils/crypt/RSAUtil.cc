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
#include <flagwld/base/CurrentThread.h>
#include <flagwld/base/Mutex.h>
    
#include <boost/shared_ptr.hpp>

#include <openssl/pem.h>
#include <openssl/crypto.h>

#include <vector>

using namespace flagwld;
using namespace flagwld::utils;
  
namespace
{
typedef boost::shared_ptr<MutexLock> MutexLockPtr;
typedef std::vector<MutexLockPtr> MutexLockPtrs;

MutexLockPtrs opensslMuextPtrs;

#pragma GCC diagnostic ignored "-Wold-style-cast"
class OpenSslThreadsObj
{
  public:
    OpenSslThreadsObj()
    {
      //CRYPTO_THREADID_set_callback(((CRYPTO_THREADID*)pthreads_thread_id);
      CRYPTO_set_id_callback(pthreads_thread_id);
      CRYPTO_set_locking_callback((void (*)(int, int ,const char*,int))pthreads_locking_callback);

      for (int i=0; i<CRYPTO_num_locks(); ++i)
      {
        MutexLockPtr ptr(new MutexLock);
        assert (ptr);
        opensslMuextPtrs.push_back(ptr);
      }
    }
    ~OpenSslThreadsObj()
    {
      opensslMuextPtrs.clear();
    }

    static unsigned long pthreads_thread_id(void)
    {
      unsigned long ret = CurrentThread::tid();
      return (ret);
    }

    static void pthreads_locking_callback(int mode, int type, const char *file, int line)
    {
      if (mode & CRYPTO_LOCK)
      {
        opensslMuextPtrs[type]->lock();
      }
      else
      {
        opensslMuextPtrs[type]->unlock();
      }
   }
};
#pragma GCC diagnostic error "-Wold-style-cast"

OpenSslThreadsObj opensslThrdObj;

class CryptoMemObj
{
  public:
    CryptoMemObj()
    {
    }
    ~CryptoMemObj()
    {
      CRYPTO_cleanup_all_ex_data();
    }
};

CryptoMemObj cryptoMemObj;
}


bool flagwld::utils::defaultRSADataCallback(unsigned char*, size_t len)
{ 
  LOG_TRACE << "RSA process ouput " << len;
  return true;
}

using namespace flagwld;
using namespace flagwld::utils;

static RSA* readPubKeyFromMem(unsigned char* pubkey, int keysize);
static RSA* readPriKeyFromMem(unsigned char* prikey, int keysize);

/***************************************** rsa pub encoder *********************************/
RSAPubEncoder::RSAPubEncoder(unsigned char*k,  int l)
           :rsa_(NULL),
            dataCallback_(defaultRSADataCallback),
            bufferSize_(kRsaBuffer),
            rsaSize_(0)
{
  if ((rsa_=readPubKeyFromMem(k, l))!=NULL)
  {
    rsaSize_ = RSA_size(rsa_);

    assert (rsaSize_>0 && rsaSize_<=bufferSize_);
  }
  else
  {
    LOG_ERROR << "RSA Publickey Encoder Init Error!"; 
  }  
}

RSAPubEncoder::~RSAPubEncoder()
{
  if(rsa_ != NULL)
  {
    RSA_free(rsa_);
    rsa_ = NULL;
  } 
}

int RSAPubEncoder::execute(unsigned char*buff, size_t len)
{
  assert (rsa_);

  if (rsa_==NULL)
  {
    LOG_ERROR << "RSA NULL Error!";
    return -1;
  } 
  
  assert (buff && len>0);

  if (buff==NULL || len<=0)
  {
    LOG_ERROR << "Input Error!";
    return -1;
  }

  int total=0;

  do
  {
    size_t modesize=rsaSize_-11;
    int in_len = static_cast<int>(len>=modesize?modesize:len);

    int ret = RSA_public_encrypt(in_len, buff+total, buffer_, rsa_, RSA_PKCS1_PADDING);
    if (ret<0)
    {
      return -1;
    }

    assert (ret == rsaSize_);

    if (!dataCallback_(buffer_, ret))
    {
      return -1;
    }

    total += in_len;
    len -= in_len;
  } while(len>0);

  return total;
}

/***************************************** rsa pri decoder *********************************/
RSAPriDecoder::RSAPriDecoder(unsigned char*k,  int l)
           :rsa_(NULL),
            dataCallback_(defaultRSADataCallback),
            bufferSize_(kRsaBuffer),
            rsaSize_(0)
{           
  if ((rsa_=readPriKeyFromMem(k, l))!=NULL)
  {
    rsaSize_ = RSA_size(rsa_);
    
    assert (rsaSize_>0 && rsaSize_<=bufferSize_);
  } 
  else
  {
    LOG_ERROR << "RSA Privatekey Decoder Init Error!";
  }  
} 

RSAPriDecoder::~RSAPriDecoder()
{
  if (rsa_!=NULL)
  {
    RSA_free(rsa_);
    rsa_ = NULL;
  } 
} 

#pragma GCC diagnostic ignored "-Wsign-compare"
int RSAPriDecoder::execute(unsigned char*buff, size_t len)
{
  assert (rsa_);
  
  if (rsa_==NULL)
  {
    LOG_ERROR << "RSA NULL Error!";
    return -1;
  } 
  
  assert (buff && len>0);
  
  if (buff==NULL || len<=0)
  {
    LOG_ERROR << "Input Error!";
    return -1;
  } 
  
  int total=0;
  
  do
  {
    int in_len = static_cast<int>(len>=rsaSize_?rsaSize_:len);
    if (in_len!=rsaSize_)
    {
      return -1;
    }
    int ret = RSA_private_decrypt(in_len, buff+total, buffer_, rsa_, RSA_PKCS1_PADDING);
    if (ret<0)
    {
      return -1;
    } 
    
    assert (ret <= rsaSize_);
    
    if (!dataCallback_(buffer_, ret))
    {
      return -1;
    } 
    
    total += in_len;
    len -= in_len;
  }while(len>0);

  return total;
}
#pragma GCC diagnostic warning "-Wsign-compare"

/***************************************** rsa pri encoder *********************************/
RSAPriEncoder::RSAPriEncoder(unsigned char*k,  int l)
           :rsa_(NULL),
            dataCallback_(defaultRSADataCallback),
            bufferSize_(kRsaBuffer),
            rsaSize_(0)
{   
  if ((rsa_=readPriKeyFromMem(k, l))!=NULL)
  {
    rsaSize_ = RSA_size(rsa_);
   
    assert (rsaSize_>0 && rsaSize_<=bufferSize_);
  }
  else
  {
    LOG_ERROR << "RSA Privatekey Encoder Init Error!";
  } 
}

RSAPriEncoder::~RSAPriEncoder()
{
  if (rsa_!=NULL)
  {
    RSA_free(rsa_);
    rsa_ = NULL;
  }
}

int RSAPriEncoder::execute(unsigned char*buff, size_t len)
{
  assert (rsa_);
 
  if (rsa_==NULL)
  {
    LOG_ERROR << "RSA NULL Error!";
    return -1;
  }
 
  assert (buff && len>0);
 
  if (buff==NULL || len<=0)
  {
    LOG_ERROR << "Input Error!";
    return -1;
  }
 
  int total=0;
 
  do
  {
    size_t modesize=rsaSize_-11;
    int in_len = static_cast<int>(len>=modesize?modesize:len);

    int ret = RSA_private_encrypt(in_len, buff+total, buffer_, rsa_, RSA_PKCS1_PADDING);
    if(ret < 0)
    {
      return -1;
    }

    assert (ret == rsaSize_);

    if(!dataCallback_(buffer_, ret))
    {
      return -1;
    }

    total += in_len;
    len -= in_len;
  }while(len>0);

  return total;
}

/***************************************** rsa pub decoder *********************************/
RSAPubDecoder::RSAPubDecoder(unsigned char*k,  int l) 
           :rsa_(NULL),
            dataCallback_(defaultRSADataCallback),
            bufferSize_(kRsaBuffer),
            rsaSize_(0)
{ 
  if((rsa_=readPubKeyFromMem(k, l))!=NULL)
  { 
    rsaSize_ = RSA_size(rsa_);

    assert (rsaSize_>0 && rsaSize_<=bufferSize_);
  }
  else
  {
    LOG_ERROR << "RSA Publickey Decoder Init Error!";
  }  
} 
RSAPubDecoder::~RSAPubDecoder()
{ 
  if(rsa_ != NULL)
  {
    RSA_free(rsa_);
    rsa_ = NULL; 
  } 
}   
    
#pragma GCC diagnostic ignored "-Wsign-compare"
int RSAPubDecoder::execute(unsigned char*buff, size_t len)
{   
  assert (rsa_);
    
  if(rsa_==NULL)
  { 
    LOG_ERROR << "RSA NULL Error!";
    return -1;
  }

  assert (buff && len>0);

  if(buff==NULL || len<=0)
  {
    LOG_ERROR << "Input Error!";
    return -1;
  }

  int total=0;

  do
  {
    int in_len = static_cast<int>(len>=rsaSize_?rsaSize_:len);
    if (in_len!=rsaSize_)
    {
      return -1;
    }
    int ret = RSA_public_decrypt(in_len, buff+total, buffer_, rsa_, RSA_PKCS1_PADDING);
    if(ret < 0)
    {
      return -1;
    }

    assert (ret <= rsaSize_);

    if(!dataCallback_(buffer_, ret))
    {
      return -1;
    }

    total += in_len;
    len -= in_len;
  } while(len>0);

  return total;
}
#pragma GCC diagnostic warning "-Wsign-compare"

/*********************** inner tools ***************************/
static RSA* readPubKeyFromMem(unsigned char* pubkey, int keysize)
{
  EVP_PKEY *pkey = NULL; 
  BIO *bio; 

  if((bio = BIO_new_mem_buf(pubkey, keysize)) == NULL)
  { 
    BIO_free(bio); 
    return NULL; 
  } 

  if((pkey = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL)) == NULL)
  { 
    BIO_free(bio); 
    return NULL; 
  } 
    
  RSA* key = EVP_PKEY_get1_RSA(pkey);
  BIO_free(bio); 
  EVP_PKEY_free(pkey);

  return key; 
}

#pragma GCC diagnostic ignored "-Wconversion"
int pass_cb(char *buf, int size, int rwflag, void *u)
{
  const char *tmp = "10023810";
  size_t len = strlen(tmp);
  memcpy(buf, tmp, len);

  fprintf(stderr, "KK: %s  %d\n", buf, size);

  return len;
}
#pragma GCC diagnostic warning "-Wconversion"

static RSA* readPriKeyFromMem(unsigned char* prikey, int keysize)
{
  EVP_PKEY *pkey = NULL;
  BIO *bio; 
  
  if((bio = BIO_new_mem_buf(prikey, keysize)) == NULL)
  {
    BIO_free(bio); 
    return NULL;
  }

  if((pkey = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL)) == NULL)
  {
    BIO_free(bio); 
    return NULL;
  } 
 
  RSA* key = EVP_PKEY_get1_RSA(pkey);
  BIO_free(bio); 
  EVP_PKEY_free(pkey);
             
  return key; 
}
