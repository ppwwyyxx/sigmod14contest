// File: debugutils.h

// Author: Yuxin Wu <ppwwyyxxc@gmail.com>


#pragma once

#if __cplusplus > 199711L
#include <iterator>
#include <type_traits>
#endif

void __m_assert_check__(bool val, const char *expr,
		const char *file, const char *func, int line);


void error_exit(const char *msg) __attribute__((noreturn));


#ifdef DEBUG

#define print_debug(fmt, ...) \
			__print_debug__(__FILE__, __func__, __LINE__, fmt, ## __VA_ARGS__)

#define m_assert(expr) \
	__m_assert_check__((expr), # expr, __FILE__, __PRETTY_FUNCTION__, __LINE__)


void __print_debug__(const char *file, const char *func, int line, const char *fmt, ...)
	__attribute__((format(printf, 4, 5)));

#include <iostream>
#define P(a) std::cout << (a)
#define PP(a) std::cout << #a << ": " << (a) << std::endl

#if __cplusplus > 199711L
#define PA(arr) \
	do { \
		std::cout << #arr << ": "; \
		auto x__long__long = (arr); \
		std::copy(begin(x__long__long), end(x__long__long), std::ostream_iterator<std::remove_reference<decltype(x__long__long)>::type::value_type>(std::cout, " ")); \
		std::cout << std::endl;  \
	} while (0)
#else
#define PA(arr)
#endif

#else

#define print_debug(fmt, ...)

#define m_assert(expr)

#define P(a)
#define PP(a)
#define PA(a)


#endif
