// src/CTools.h - common
#pragma once // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)

#undef _WIN32
#define PTHREAD_COND_INITIALIZER {{0}}
typedef struct {
	int foo;
} priority_queue_t;
typedef struct {
    priority_queue_t queue; 
} pthread_cond_t;
typedef struct {
	int bar;
} pthread_condattr_t;
int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);

struct CTools {
#ifdef _WIN32
	static constexpr HANDLE c_invalid = INVALID_HANDLE_VALUE;
	typedef HANDLE handle_t;
#else
	static constexpr pthread_cond_t c_invalid = PTHREAD_COND_INITIALIZER;
	typedef pthread_cond_t handle_t;
#endif
	static handle_t CopyHandle(handle_t input) {
		handle_t out = c_invalid;
#ifdef _WIN32
		HANDLE process = GetCurrentProcess( );
		::DuplicateHandle( process, 
				input, 
				process,
				&out, 
				0,
				FALSE,
				DUPLICATE_SAME_ACCESS
			);
#else
		// ...
#endif
		return out;
	}
	static void CloseAndInvalidateHandle(handle_t &handle) {
#ifdef _WIN32
		if ( c_invalid == handle || !handle )
			return;
		CloseHandle( handle );
#else
		pthread_cond_destroy( &handle );
#endif
		handle = c_invalid;
	}
};
