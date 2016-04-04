#ifndef __YIJIAN_MACRO_H__
#define __YIJIAN_MACRO_H__

#include <stdexcept>
#include <string.h>
#include <string>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#define likely(x) __builtin_expect(!!(x),1)
#define unlikely(x) __builtin_expect(!!(x),0)
//copy from assert.h
__BEGIN_DECLS

/* This prints an "Assertion failed" message and aborts.  */
extern void __assert_fail (const char *__assertion, const char *__file,
			   unsigned int __line, const char *__function)
     __THROW __attribute__ ((__noreturn__));

/* Likewise, but prints the error text for ERRNUM.  */
extern void __assert_perror_fail (int __errnum, const char *__file,
				  unsigned int __line, const char *__function)
     __THROW __attribute__ ((__noreturn__));


/* The following is not at all used here but needed for standard
   compliance.  */
extern void __assert (const char *__assertion, const char *__file, int __line)
     __THROW __attribute__ ((__noreturn__));


__END_DECLS
#define CKERROR(errno) ( { \
			__typeof__ (errno) err = (errno);\
			if (__builtin_expect(err != 0, 0)) \
			__assert_perror_fail((errno), __FILE__, __LINE__, __func__);\
		})
#ifdef DEBUG
#define log(...) printf("file|%s|line:%d->%s:%s\n...\n",__FILE__, __LINE__, __func__)
#else
#define log(charp)
#endif
namespace yijian {

class noncopyable {
public:
	noncopyable() = default;
	noncopyable(const noncopyable &) = delete;
	noncopyable& operator=(const noncopyable&) = delete;
	noncopyable(noncopyable &&) = default;
	noncopyable& operator=(noncopyable&&) = default;
	virtual ~noncopyable() = default;
};

}

#endif


