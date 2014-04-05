//File: fast_read.h
//Date: Sat Apr 05 15:22:47 2014 +0800
//Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#pragma once


namespace {
	const int BUFFER_LEN = 1024 * 1024 * 4;
}

#define safe_open(fname) \
	FILE* fin = fopen((fname).c_str(), "r"); \
	m_assert(fin != NULL);

#define PTR_NEXT() \
{ \
	ptr ++; \
	if (ptr == buf_end) \
	{ \
		ptr = buffer; \
		buf_end = buffer + fread(buffer, 1, BUFFER_LEN, fin); \
	} \
}

// only read non-negative int
#define READ_INT(_x_) \
{ \
	while (*ptr < '0' || *ptr > '9') \
		PTR_NEXT(); \
	int _n_ = 0; \
	while (*ptr >= '0' && *ptr <= '9') \
	{ \
		_n_ = _n_ * 10 + *ptr - '0'; \
		PTR_NEXT(); \
	} \
	(_x_) = (_n_); \
}

#define READ_ULL(_x_) \
{ \
	while (*ptr < '0' || *ptr > '9') \
		PTR_NEXT(); \
	unsigned long long _n_ = 0; \
	while (*ptr >= '0' && *ptr <= '9') \
	{ \
		_n_ = _n_ * 10 + *ptr - '0'; \
		PTR_NEXT(); \
	} \
	(_x_) = (_n_); \
}

#define READ_STR(_s_) \
{ \
	char *_p_ = (_s_); \
	while (*ptr != '|') \
	{ \
		*(_p_ ++) = *ptr; \
		PTR_NEXT(); \
	} \
	*_p_ = 0; \
}

#define READ_TILL_EOL() \
	while (*ptr != '\n') PTR_NEXT();

#define PTR_NEXT_s(s) \
{ \
	(ptr ## s) ++; \
	if (ptr ## s == buf_end ## s) \
	{ \
		ptr ## s = buffer ## s; \
		buf_end ## s = buffer ## s + fread(buffer ## s, 1, BUFFER_LEN, fp ## s); \
	} \
}

#define READ_INT_s(s, _x_) \
{ \
	while (*(ptr ## s) < '0' || *(ptr ## s) > '9') \
		PTR_NEXT_s(s); \
	int _n_ = 0; \
	while (*(ptr ## s) >= '0' && *(ptr ## s) <= '9') \
	{ \
		_n_ = _n_ * 10 + *(ptr ## s) - '0'; \
		PTR_NEXT_s(s); \
	} \
	(_x_) = (_n_); \
}

#define READ_TILL_EOL_s(s) \
	while (*(ptr ## s) != '\n') PTR_NEXT_s(s);


#define MMAP_READ_TILL_EOL() \
	{ \
		do { ptr++; } while (*ptr != '\n'); \
		ptr ++; \
	}
