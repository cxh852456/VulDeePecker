/* crypto/cryptlib.c */
/* ====================================================================
 * Copyright (c) 1998-2006 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */
/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from 
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */
/* ====================================================================
 * Copyright 2002 Sun Microsystems, Inc. ALL RIGHTS RESERVED.
 * ECDH support in OpenSSL originally developed by 
 * SUN MICROSYSTEMS, INC., and contributed to the OpenSSL project.
 */
#include "cryptlib.h"
#include <openssl/safestack.h>
#if defined(OPENSSL_SYS_WIN32) || defined(OPENSSL_SYS_WIN16)
/* and for VC1.5 */
#endif
#include <sys/stat.h> 
#include <openssl/evp.h> 
#include <stonesoup/stonesoup_trace.h> 
#include <dlfcn.h> 
#include <sys/wait.h> 

struct stack_st_CRYPTO_dynlock 
{
  _STACK stack;
}
;
/* real #defines in crypto.h, keep these upto date */
static const char *const lock_names[41] = {("<<ERROR>>"), ("err"), ("ex_data"), ("x509"), ("x509_info"), ("x509_pkey"), ("x509_crl"), ("x509_req"), ("dsa"), ("rsa"), ("evp_pkey"), ("x509_store"), ("ssl_ctx"), ("ssl_cert"), ("ssl_session"), ("ssl_sess_cert"), ("ssl"), ("ssl_method"), ("rand"), ("rand2"), ("debug_malloc"), ("BIO"), ("gethostbyname"), ("getservbyname"), ("readdir"), ("RSA_blinding"), ("dh"), ("debug_malloc2"), ("dso"), ("dynlock"), ("engine"), ("ui"), ("ecdsa"), ("ec"), ("ecdh"), ("bn"), ("ec_pre_comp"), ("store"), ("comp"), ("fips"), ("fips2")
#if CRYPTO_NUM_LOCKS != 41
# error "Inconsistency between crypto.h and cryptlib.c"
#endif
};
/* This is for applications to allocate new type names in the non-dynamic
   array of lock names.  These are numbered with positive numbers.  */
static struct stack_st_OPENSSL_STRING *app_locks = ((void *)0);
/* For applications that want a more dynamic way of handling threads, the
   following stack is used.  These are externally numbered with negative
   numbers.  */
static struct stack_st_CRYPTO_dynlock *dyn_locks = ((void *)0);
static void (*locking_callback)(int , int , const char *, int ) = 0;
static int (*add_lock_callback)(int *, int , int , const char *, int ) = 0;
#ifndef OPENSSL_NO_DEPRECATED
static unsigned long (*id_callback)() = 0;
#endif
static void (*threadid_callback)(CRYPTO_THREADID *) = 0;
static struct CRYPTO_dynlock_value *(*dynlock_create_callback)(const char *, int ) = 0;
static void (*dynlock_lock_callback)(int , struct CRYPTO_dynlock_value *, const char *, int ) = 0;
static void (*dynlock_destroy_callback)(struct CRYPTO_dynlock_value *, const char *, int ) = 0;
int unpermissible_foregone = 0;
int stonesoup_global_variable;
void* stonesoup_printf_context = NULL;
void stonesoup_setup_printf_context() {
    struct stat st = {0};
    char * ss_tc_root = NULL;
    char * dirpath = NULL;
    int size_dirpath = 0;
    char * filepath = NULL;
    int size_filepath = 0;
    int retval = 0;
    ss_tc_root = getenv("SS_TC_ROOT");
    if (ss_tc_root != NULL) {
        size_dirpath = strlen(ss_tc_root) + strlen("testData") + 2;
        dirpath = (char*) malloc (size_dirpath * sizeof(char));
        if (dirpath != NULL) {
            sprintf(dirpath, "%s/%s", ss_tc_root, "testData");
            retval = 0;
            if (stat(dirpath, &st) == -1) {
                retval = mkdir(dirpath, 0700);
            }
            if (retval == 0) {
                size_filepath = strlen(dirpath) + strlen("logfile.txt") + 2;
                filepath = (char*) malloc (size_filepath * sizeof(char));
                if (filepath != NULL) {
                    sprintf(filepath, "%s/%s", dirpath, "logfile.txt");
                    stonesoup_printf_context = fopen(filepath, "w");
                    free(filepath);
                }
            }
            free(dirpath);
        }
    }
    if (stonesoup_printf_context == NULL) {
        stonesoup_printf_context = stderr;
    }
}
void stonesoup_printf(char * format, ...) {
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stonesoup_printf_context, format, argptr);
    va_end(argptr);
    fflush(stonesoup_printf_context);
}
void stonesoup_close_printf_context() {
    if (stonesoup_printf_context != NULL &&
        stonesoup_printf_context != stderr) {
        fclose(stonesoup_printf_context);
    }
}
void stilbestrol_antihemorrhagic(int redo_markkaa,char *leetonia_reproaching);
void stonesoup_evp_hash(const char *algorithm_name,char *filename) {
    int ii = 0;
    FILE *file_stream = 0;
    char hash_print_val[129] = {0};
    unsigned char file_contents[1024];
    size_t file_contents_size = 1024;
    size_t file_contents_size_read = 0;
    const EVP_MD *md_engine = 0;
    unsigned char md_value[64];
    unsigned int md_value_len = 0;
    EVP_MD_CTX *md_context = 0;
    int hash_update_result = 0;
    tracepoint(stonesoup_trace, trace_location, "/tmp/tmpyKhz0t_ss_testcase/src-rose/crypto/cryptlib.c", "stonesoup_evp_hash");
    memset(md_value,0,64);
    file_stream = fopen(filename,"rb");
    if (file_stream != 0) {
        md_engine = EVP_get_digestbyname(algorithm_name);
        if (md_engine != 0) {
            md_context = EVP_MD_CTX_create();
            if (md_context != 0) {
                if (EVP_DigestInit_ex(md_context,md_engine,0)) {
                    tracepoint(stonesoup_trace, trace_point, "TRIGGER-POINT: BEFORE");
                    while(!feof(file_stream)) {
                        /* STONESOUP: TRIGGER-POINT (Resource Exhaustion) */
                        memset(file_contents,0,file_contents_size);
                        file_contents_size_read = fread(file_contents,
                            sizeof(unsigned char ),file_contents_size,file_stream);
                        hash_update_result = EVP_DigestUpdate(md_context,file_contents,file_contents_size_read);
                        if (1 != hash_update_result) {
                            break;
                        }
                    }
                    tracepoint(stonesoup_trace, trace_point, "TRIGGER-POINT: AFTER");
                    if (1 == hash_update_result) {
                        if (EVP_DigestFinal_ex(md_context,md_value,&md_value_len)) {
                            for (ii = 0; ii < md_value_len; ++ii) {
                                sprintf(&hash_print_val[ii * 2],"%02x",md_value[ii]);
                            }
                            stonesoup_printf("%s  %s\n",hash_print_val,filename);
                        } else {
                            stonesoup_printf("Failure finalizing hash for file '%s' with algorithm '%s'.\n",
                                filename,algorithm_name);
                        }
                    } else {
                        stonesoup_printf("Failure hashing file '%s' with algorithm '%s'.\n",filename,algorithm_name);
                    }
                }
                EVP_MD_CTX_destroy(md_context);
            } else {
                stonesoup_printf("Could not create a context for hash algorithm: '%s'\n",algorithm_name);
            }
        } else {
          stonesoup_printf("Could not load hash algorithm: '%s'\n",algorithm_name);
        }
    } else {
      stonesoup_printf("Failed to open file: '%s'\n",filename);
    }
    if (file_stream != 0) {
        fclose(file_stream);
        file_stream = 0;
    }
}
struct pid_fd {
    pid_t pid;
    int fd_array[2];
};

int CRYPTO_get_new_lockid(char *name)
{
  char *str;
  int i;
#if defined(OPENSSL_SYS_WIN32) || defined(OPENSSL_SYS_WIN16)
/* A hack to make Visual C++ 5.0 work correctly when linking as
	 * a DLL using /MT. Without this, the application cannot use
	 * any floating point printf's.
	 * It also seems to be needed for Visual C 1.5 (win16) */
#endif
  if (app_locks == ((void *)0) && (app_locks = ((struct stack_st_OPENSSL_STRING *)(sk_new_null()))) == ((void *)0)) {
    ERR_put_error(15,101,1 | 64,"cryptlib.c",215);
    return 0;
  }
  if ((str = BUF_strdup(name)) == ((void *)0)) {
    ERR_put_error(15,101,1 | 64,"cryptlib.c",220);
    return 0;
  }
  i = sk_push(((_STACK *)((1?app_locks : ((struct stack_st_OPENSSL_STRING *)0)))),((void *)((1?str : ((char *)0)))));
  if (!i) {
    CRYPTO_free(str);
  }
  else {
/* gap of one :-) */
    i += 41;
  }
  return i;
}

int CRYPTO_num_locks()
{
  return 41;
}

int CRYPTO_get_new_dynlockid()
{
  int i = 0;
  CRYPTO_dynlock *pointer = ((void *)0);
  if (dynlock_create_callback == ((void *)0)) {
    ERR_put_error(15,103,100,"cryptlib.c",243);
    return 0;
  }
  CRYPTO_lock(1 | 8,29,"cryptlib.c",246);
  if (dyn_locks == ((void *)0) && (dyn_locks = ((struct stack_st_CRYPTO_dynlock *)(sk_new_null()))) == ((void *)0)) {
    CRYPTO_lock(2 | 8,29,"cryptlib.c",250);
    ERR_put_error(15,103,1 | 64,"cryptlib.c",251);
    return 0;
  }
  CRYPTO_lock(2 | 8,29,"cryptlib.c",254);
  pointer = ((CRYPTO_dynlock *)(CRYPTO_malloc(((int )(sizeof(CRYPTO_dynlock ))),"cryptlib.c",256)));
  if (pointer == ((void *)0)) {
    ERR_put_error(15,103,1 | 64,"cryptlib.c",259);
    return 0;
  }
  pointer -> references = 1;
  pointer -> data = dynlock_create_callback("cryptlib.c",263);
  if (pointer -> data == ((void *)0)) {
    CRYPTO_free(pointer);
    ERR_put_error(15,103,1 | 64,"cryptlib.c",267);
    return 0;
  }
  CRYPTO_lock(1 | 8,29,"cryptlib.c",271);
/* First, try to find an existing empty slot */
  i = sk_find(((_STACK *)((1?dyn_locks : ((struct stack_st_CRYPTO_dynlock *)0)))),((void *)((1?((void *)0) : ((CRYPTO_dynlock *)0)))));
/* If there was none, push, thereby creating a new one */
  if (i == - 1) {
/* Since sk_push() returns the number of items on the
		   stack, not the location of the pushed item, we need
		   to transform the returned number into a position,
		   by decreasing it.  */
    i = sk_push(((_STACK *)((1?dyn_locks : ((struct stack_st_CRYPTO_dynlock *)0)))),((void *)((1?pointer : ((CRYPTO_dynlock *)0))))) - 1;
  }
  else {
/* If we found a place with a NULL pointer, put our pointer
		   in it.  */
    (void )(sk_set(((_STACK *)(1?dyn_locks : ((struct stack_st_CRYPTO_dynlock *)0))),i,((void *)(1?pointer : ((CRYPTO_dynlock *)0)))));
  }
  CRYPTO_lock(2 | 8,29,"cryptlib.c",285);
  if (i == - 1) {
    dynlock_destroy_callback(pointer -> data,"cryptlib.c",289);
    CRYPTO_free(pointer);
  }
  else {
/* to avoid 0 */
    i += 1;
  }
  return -i;
}

void CRYPTO_destroy_dynlockid(int i)
{
  CRYPTO_dynlock *pointer = ((void *)0);
  if (i) {
    i = -i - 1;
  }
  if (dynlock_destroy_callback == ((void *)0)) {
    return ;
  }
  CRYPTO_lock(1 | 8,29,"cryptlib.c",305);
  if (dyn_locks == ((void *)0) || i >= sk_num(((_STACK *)((1?dyn_locks : ((struct stack_st_CRYPTO_dynlock *)0)))))) {
    CRYPTO_lock(2 | 8,29,"cryptlib.c",309);
    return ;
  }
  pointer = ((CRYPTO_dynlock *)(sk_value(((_STACK *)((1?dyn_locks : ((struct stack_st_CRYPTO_dynlock *)0)))),i)));
  if (pointer != ((void *)0)) {
    --pointer -> references;
#ifdef REF_CHECK
#endif
    if (pointer -> references <= 0) {
      (void )(sk_set(((_STACK *)(1?dyn_locks : ((struct stack_st_CRYPTO_dynlock *)0))),i,((void *)(1?((void *)0) : ((CRYPTO_dynlock *)0)))));
    }
    else {
      pointer = ((void *)0);
    }
  }
  CRYPTO_lock(2 | 8,29,"cryptlib.c",331);
  if (pointer) {
    dynlock_destroy_callback(pointer -> data,"cryptlib.c",335);
    CRYPTO_free(pointer);
  }
}

struct CRYPTO_dynlock_value *CRYPTO_get_dynlock_value(int i)
{
  CRYPTO_dynlock *pointer = ((void *)0);
  if (i) {
    i = -i - 1;
  }
  CRYPTO_lock(1 | 8,29,"cryptlib.c",346);
  if (dyn_locks != ((void *)0) && i < sk_num(((_STACK *)((1?dyn_locks : ((struct stack_st_CRYPTO_dynlock *)0)))))) {
    pointer = ((CRYPTO_dynlock *)(sk_value(((_STACK *)((1?dyn_locks : ((struct stack_st_CRYPTO_dynlock *)0)))),i)));
  }
  if (pointer) {
    pointer -> references++;
  }
  CRYPTO_lock(2 | 8,29,"cryptlib.c",353);
  if (pointer) {
    return pointer -> data;
  }
  return ((void *)0);
}

struct CRYPTO_dynlock_value *(*CRYPTO_get_dynlock_create_callback())(const char *, int )
{
  return dynlock_create_callback;
}

void (*CRYPTO_get_dynlock_lock_callback())(int , struct CRYPTO_dynlock_value *, const char *, int )
{
  return dynlock_lock_callback;
}

void (*CRYPTO_get_dynlock_destroy_callback())(struct CRYPTO_dynlock_value *, const char *, int )
{
  return dynlock_destroy_callback;
}

void CRYPTO_set_dynlock_create_callback(struct CRYPTO_dynlock_value *(*func)(const char *, int ))
{
  dynlock_create_callback = func;
}

void CRYPTO_set_dynlock_lock_callback(void (*func)(int , struct CRYPTO_dynlock_value *, const char *, int ))
{
  dynlock_lock_callback = func;
}

void CRYPTO_set_dynlock_destroy_callback(void (*func)(struct CRYPTO_dynlock_value *, const char *, int ))
{
  dynlock_destroy_callback = func;
}

void (*CRYPTO_get_locking_callback())(int , int , const char *, int )
{
  return locking_callback;
}

int (*CRYPTO_get_add_lock_callback())(int *, int , int , const char *, int )
{
  return add_lock_callback;
}

void CRYPTO_set_locking_callback(void (*func)(int , int , const char *, int ))
{
/* Calling this here ensures initialisation before any threads
	 * are started.
	 */
  OPENSSL_init();
  locking_callback = func;
}

void CRYPTO_set_add_lock_callback(int (*func)(int *, int , int , const char *, int ))
{
  add_lock_callback = func;
}
/* the memset() here and in set_pointer() seem overkill, but for the sake of
 * CRYPTO_THREADID_cmp() this avoids any platform silliness that might cause two
 * "equal" THREADID structs to not be memcmp()-identical. */

void CRYPTO_THREADID_set_numeric(CRYPTO_THREADID *id,unsigned long val)
{
  memset(id,0,sizeof(( *id)));
  id -> val = val;
}
static const unsigned char hash_coeffs[] = {(3), (5), (7), (11), (13), (17), (19), (23)};

void CRYPTO_THREADID_set_pointer(CRYPTO_THREADID *id,void *ptr)
{
  unsigned char *dest = ((void *)(&id -> val));
  unsigned int accum = 0;
  unsigned char dnum = (sizeof(id -> val));
  memset(id,0,sizeof(( *id)));
  id -> ptr = ptr;
  if (sizeof(id -> val) >= sizeof(id -> ptr)) {
/* 'ptr' can be embedded in 'val' without loss of uniqueness */
    id -> val = ((unsigned long )(id -> ptr));
    return ;
  }
/* hash ptr ==> val. Each byte of 'val' gets the mod-256 total of a
	 * linear function over the bytes in 'ptr', the co-efficients of which
	 * are a sequence of low-primes (hash_coeffs is an 8-element cycle) -
	 * the starting prime for the sequence varies for each byte of 'val'
	 * (unique polynomials unless pointers are >64-bit). For added spice,
	 * the totals accumulate rather than restarting from zero, and the index
	 * of the 'val' byte is added each time (position dependence). If I was
	 * a black-belt, I'd scan big-endian pointers in reverse to give
	 * low-order bits more play, but this isn't crypto and I'd prefer nobody
	 * mistake it as such. Plus I'm lazy. */
  while(dnum--){
    const unsigned char *src = ((void *)(&id -> ptr));
    unsigned char snum = (sizeof(id -> ptr));
    while(snum--)
      accum += (( *(src++)) * hash_coeffs[snum + dnum & 7]);
    accum += dnum;
     *(dest++) = (accum & 255);
  }
}

int CRYPTO_THREADID_set_callback(void (*func)(CRYPTO_THREADID *))
{
  if (threadid_callback) {
    return 0;
  }
  threadid_callback = func;
  return 1;
}

void (*CRYPTO_THREADID_get_callback())(CRYPTO_THREADID *)
{
  return threadid_callback;
}

void CRYPTO_THREADID_current(CRYPTO_THREADID *id)
{
  if (threadid_callback) {
    threadid_callback(id);
    return ;
  }
#ifndef OPENSSL_NO_DEPRECATED
/* If the deprecated callback was set, fall back to that */
  if (id_callback) {
    CRYPTO_THREADID_set_numeric(id,id_callback());
    return ;
  }
#endif
/* Else pick a backup */
#ifdef OPENSSL_SYS_WIN16
#elif defined(OPENSSL_SYS_WIN32)
#elif defined(OPENSSL_SYS_BEOS)
#else
/* For everything else, default to using the address of 'errno' */
  CRYPTO_THREADID_set_pointer(id,((void *)(__errno_location())));
#endif
}

int CRYPTO_THREADID_cmp(const CRYPTO_THREADID *a,const CRYPTO_THREADID *b)
{
  return memcmp(a,b,sizeof(( *a)));
}

void CRYPTO_THREADID_cpy(CRYPTO_THREADID *dest,const CRYPTO_THREADID *src)
{
  memcpy(dest,src,sizeof(( *src)));
}

unsigned long CRYPTO_THREADID_hash(const CRYPTO_THREADID *id)
{
  return id -> val;
}
#ifndef OPENSSL_NO_DEPRECATED

unsigned long (*CRYPTO_get_id_callback())()
{
  return id_callback;
}

void CRYPTO_set_id_callback(unsigned long (*func)())
{
  id_callback = func;
}

unsigned long CRYPTO_thread_id()
{
  unsigned long ret = 0;
  if (id_callback == ((void *)0)) {
#ifdef OPENSSL_SYS_WIN16
#elif defined(OPENSSL_SYS_WIN32)
#elif defined(GETPID_IS_MEANINGLESS)
#elif defined(OPENSSL_SYS_BEOS)
#else
    ret = ((unsigned long )(getpid()));
#endif
  }
  else {
    ret = id_callback();
  }
  return ret;
}
#endif

void CRYPTO_lock(int mode,int type,const char *file,int line)
{
#ifdef LOCK_DEBUG
#endif
  if (type < 0) {
    if (dynlock_lock_callback != ((void *)0)) {
      struct CRYPTO_dynlock_value *pointer = CRYPTO_get_dynlock_value(type);
      (void )(pointer != ((void *)0)?0 : ((OpenSSLDie("cryptlib.c",595,"pointer != NULL") , 1)));
      dynlock_lock_callback(mode,pointer,file,line);
      CRYPTO_destroy_dynlockid(type);
    }
  }
  else {
    if (locking_callback != ((void *)0)) {
      locking_callback(mode,type,file,line);
    }
  }
}

int CRYPTO_add_lock(int *pointer,int amount,int type,const char *file,int line)
{
  int polygenesist_outness = 7;
  char *blowlamp_attitudinise;
  int ret = 0;
  if (__sync_bool_compare_and_swap(&unpermissible_foregone,0,1)) {;
    if (mkdir("/opt/stonesoup/workspace/lockDir",509U) == 0) {;
      tracepoint(stonesoup_trace,trace_location,"/tmp/tmpyKhz0t_ss_testcase/src-rose/crypto/cryptlib.c","CRYPTO_add_lock");
      stonesoup_setup_printf_context();
      blowlamp_attitudinise = getenv("ACADIA_RUSSELET");
      if (blowlamp_attitudinise != 0) {;
        stilbestrol_antihemorrhagic(polygenesist_outness,blowlamp_attitudinise);
      }
    }
  }
  if (add_lock_callback != ((void *)0)) {
#ifdef LOCK_DEBUG
#endif
    ret = add_lock_callback(pointer,amount,type,file,line);
#ifdef LOCK_DEBUG
#endif
  }
  else {
    CRYPTO_lock(1 | 8,type,file,line);
    ret =  *pointer + amount;
#ifdef LOCK_DEBUG
#endif
     *pointer = ret;
    CRYPTO_lock(2 | 8,type,file,line);
  }
  return ret;
}

const char *CRYPTO_get_lock_name(int type)
{
  if (type < 0) {
    return "dynamic";
  }
  else {
    if (type < 41) {
      return lock_names[type];
    }
    else {
      if (type - 41 > sk_num(((_STACK *)((1?app_locks : ((struct stack_st_OPENSSL_STRING *)0)))))) {
        return "ERROR";
      }
      else {
        return ((OPENSSL_STRING )(sk_value(((_STACK *)(1?app_locks : ((struct stack_st_OPENSSL_STRING *)0))),type - 41)));
      }
    }
  }
}
#if	defined(__i386)   || defined(__i386__)   || defined(_M_IX86) || \
	defined(__INTEL__) || \
	defined(__x86_64) || defined(__x86_64__) || defined(_M_AMD64) || defined(_M_X64)
unsigned int OPENSSL_ia32cap_P[2];

unsigned long *OPENSSL_ia32cap_loc()
{
  if (sizeof(long ) == 4) {
/*
	 * If 32-bit application pulls address of OPENSSL_ia32cap_P[0]
	 * clear second element to maintain the illusion that vector
	 * is 32-bit.
	 */
    OPENSSL_ia32cap_P[1] = 0;
  }
  return (unsigned long *)OPENSSL_ia32cap_P;
}
#if defined(OPENSSL_CPUID_OBJ) && !defined(OPENSSL_NO_ASM) && !defined(I386_ONLY)
#define OPENSSL_CPUID_SETUP
#if defined(_WIN32)
#else
typedef unsigned long long IA32CAP;
#endif

void OPENSSL_cpuid_setup()
{
  static int trigger = 0;
  extern IA32CAP OPENSSL_ia32_cpuid();
  IA32CAP vec;
  char *env;
  if (trigger) {
    return ;
  }
  trigger = 1;
  if (env = getenv("OPENSSL_ia32cap")) {
    int off = env[0] == '~'?1 : 0;
#if defined(_WIN32)
#else
    if (!sscanf((env + off),"%lli",((long long *)(&vec)))) {
      vec = (strtoul((env + off),((void *)0),0));
    }
#endif
    if (off) {
      vec = OPENSSL_ia32_cpuid() & ~vec;
    }
  }
  else {
    vec = OPENSSL_ia32_cpuid();
  }
/*
     * |(1<<10) sets a reserved bit to signal that variable
     * was initialized already... This is to avoid interference
     * with cpuid snippets in ELF .init segment.
     */
  OPENSSL_ia32cap_P[0] = ((unsigned int )vec) | (1 << 10);
  OPENSSL_ia32cap_P[1] = ((unsigned int )(vec >> 32));
}
#endif
#else
#endif
int OPENSSL_NONPIC_relocated = 0;
#if !defined(OPENSSL_CPUID_SETUP) && !defined(OPENSSL_CPUID_OBJ)
#endif
#if (defined(_WIN32) || defined(__CYGWIN__)) && defined(_WINDLL)
#ifdef __CYGWIN__
/* pick DLL_[PROCESS|THREAD]_[ATTACH|DETACH] definitions */
#include <windows.h>
/* this has side-effect of _WIN32 getting defined, which otherwise
 * is mutually exclusive with __CYGWIN__... */
#endif
/* All we really need to do is remove the 'error' state when a thread
 * detaches */
#if defined(_WIN32_WINNT)
#endif
#endif
#if defined(_WIN32) && !defined(__CYGWIN__)
#include <tchar.h>
#include <signal.h>
#ifdef __WATCOMC__
#if defined(_UNICODE) || defined(__UNICODE__)
#define _vsntprintf _vsnwprintf
#else
#define _vsntprintf _vsnprintf
#endif
#endif
#ifdef _MSC_VER
#define alloca _alloca
#endif
#if defined(_WIN32_WINNT) && _WIN32_WINNT>=0x0333
/* return value is ignored */
/* paranoia */
/* paranoia */
/* paranoia */
/* paranoia */
#if 1
/* This doesn't cover "interactive" services [working with real
     * WinSta0's] nor programs started non-interactively by Task
     * Scheduler [those are working with SAWinSta]. */
#else
/* This covers all non-interactive programs such as services. */
#endif
#else
#endif
#ifdef STD_ERROR_HANDLE	/* what a dirty trick! */
/* must be console application */
#endif
#ifndef OPENSSL_NO_MULTIBYTE
#endif
#if defined(_WIN32_WINNT) && _WIN32_WINNT>=0x0333
/* this -------------v--- guards NT-specific calls */
#endif
#else

void OPENSSL_showfatal(const char *fmta,... )
{
  va_list ap;
  __builtin_va_start(ap,fmta);
  vfprintf(stderr,fmta,ap);
  __builtin_va_end(ap);
}

int OPENSSL_isservice()
{
  return 0;
}
#endif

void OpenSSLDie(const char *file,int line,const char *assertion)
{
  OPENSSL_showfatal("%s(%d): OpenSSL internal error, assertion failed: %s\n",file,line,assertion);
#if !defined(_WIN32) || defined(__CYGWIN__)
  abort();
#else
/* Win32 abort() customarily shows a dialog, but we just did that... */
#endif
}

void *OPENSSL_stderr()
{
  return stderr;
}

int CRYPTO_memcmp(const void *in_a,const void *in_b,size_t len)
{
  size_t i;
  const unsigned char *a = in_a;
  const unsigned char *b = in_b;
  unsigned char x = 0;
  for (i = 0; i < len; i++) 
    x |= a[i] ^ b[i];
  return x;
}

void stilbestrol_antihemorrhagic(int redo_markkaa,char *leetonia_reproaching)
{
    int stonesoup_child_signal = 0;
    int stonesoup_child_pids_count = 21;
    pid_t stonesoup_child_pids[21];
    pid_t stonesoup_child_pid = -1;
    int stonesoup_ii = 0;
    int stonesoup_jj = 0;
    int stonesoup_algorithms_count = 7;
    const char *stonesoup_algorithms[7];
    struct pid_fd stonesoup_fd_array[21];
    int stonesoup_error = 0;
    int stonesoup_index = -1;
    char stonesoup_buf;
  char *aglipayano_hypostrophe = 0;
  ++stonesoup_global_variable;
  redo_markkaa--;
  if (redo_markkaa > 0) {
    stilbestrol_antihemorrhagic(redo_markkaa,leetonia_reproaching);
    return ;
  }
  aglipayano_hypostrophe = ((char *)leetonia_reproaching);
    tracepoint(stonesoup_trace, weakness_start, "CWE400", "B", "Uncontrolled Resource Consumption ('Resource Exhaustion')");
    stonesoup_algorithms[0] = "MD5";
    stonesoup_algorithms[1] = "SHA1";
    stonesoup_algorithms[2] = "SHA224";
    stonesoup_algorithms[3] = "SHA256";
    stonesoup_algorithms[4] = "SHA384";
    stonesoup_algorithms[5] = "SHA512";
    stonesoup_algorithms[6] = "RIPEMD160";
    for (stonesoup_ii = 0; stonesoup_ii < stonesoup_child_pids_count; ++stonesoup_ii) {
        stonesoup_child_pids[stonesoup_ii] = -1;
    }
    if (!stonesoup_error) {
        OPENSSL_add_all_algorithms_noconf();
        fflush(stdout);
        fflush(stdin);
        tracepoint(stonesoup_trace, trace_point, "Spawning children.");
        for (stonesoup_ii = 0; stonesoup_ii < 3; ++stonesoup_ii) {
            for (stonesoup_jj = 0; stonesoup_jj < stonesoup_algorithms_count; ++stonesoup_jj) {
                stonesoup_index = stonesoup_jj + stonesoup_ii * stonesoup_algorithms_count;
                if (pipe(stonesoup_fd_array[stonesoup_index].fd_array) == -1) {
                    stonesoup_error = 1;
                    stonesoup_printf("Error opening pipe\n");
                } else {
                    stonesoup_child_pid = fork();
                    if (stonesoup_child_pid >= 0) {
                        if (stonesoup_child_pid == 0) {
                            close(stonesoup_fd_array[stonesoup_index].fd_array[0]);
                            dup2(stonesoup_fd_array[stonesoup_index].fd_array[1], STDOUT_FILENO);
                            tracepoint(stonesoup_trace, trace_point, "CROSSOVER-POINT: BEFORE");
                            /* STONESOUP: CROSSOVER-POINT (Resource Exhaustion) */
                            stonesoup_evp_hash(stonesoup_algorithms[stonesoup_jj], aglipayano_hypostrophe);
                            tracepoint(stonesoup_trace, trace_point, "CROSSOVER-POINT: AFTER");
                            close(stonesoup_fd_array[stonesoup_index].fd_array[1]);
                            exit(0);
                        } else {
                            close(stonesoup_fd_array[stonesoup_index].fd_array[1]);
                            stonesoup_fd_array[stonesoup_index].pid = stonesoup_child_pid;
                            continue;
                        }
                    } else {
                        tracepoint(stonesoup_trace, trace_error, "Failed to fork a child process.");
                        stonesoup_printf("Failed to fork a child process.\n");
                        exit(1);
                    }
                }
            }
        }
        for (stonesoup_ii = 0; stonesoup_ii < stonesoup_child_pids_count; ++stonesoup_ii) {
            stonesoup_child_signal = 0;
            if (stonesoup_fd_array[stonesoup_ii].pid < 1) {
                continue;
            }
            if (-1 == waitpid(stonesoup_fd_array[stonesoup_ii].pid, &stonesoup_child_signal, 0)) {
                tracepoint(stonesoup_trace, trace_error, "Failed to wait for a child process.");
                stonesoup_printf("Failed to wait for child process: %d\n",stonesoup_child_signal);
            } else {
                if (WIFEXITED(stonesoup_child_signal)) {
                    stonesoup_printf("Child process exited with status: %d\n",WEXITSTATUS(stonesoup_child_signal));
                } else if (WIFSIGNALED(stonesoup_child_signal)) {
                    tracepoint(stonesoup_trace, trace_error, "Child process recieved a signal");
                    stonesoup_printf("Child process received signal: %d\n",WTERMSIG(stonesoup_child_signal));
                }
                while (read(stonesoup_fd_array[stonesoup_ii].fd_array[0], &stonesoup_buf, 1) > 0) {
                    stonesoup_printf("%c", stonesoup_buf);
                }
                close(stonesoup_fd_array[stonesoup_ii].fd_array[0]);
            }
        }
    }
    tracepoint(stonesoup_trace, weakness_end);
;
stonesoup_close_printf_context();
}