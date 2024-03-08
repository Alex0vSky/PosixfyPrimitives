// src/CTools.h - common
#pragma once // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)

#undef _WIN32
#define PTHREAD_COND_INITIALIZER {{0}}
struct priority_queue_t {
	int foo;
};
struct pthread_condattr_t {
	int bar;
};
struct rt_ipc_object {
	int foo;
};
struct pthread_mutex_t {
	int foo;
};
struct pthread_mutexattr_t {
	int foo;
};
struct rt_semaphore  { 
	// Унаследовано от класса ipc_object
   rt_ipc_object parent;
   // Значение семафора
   uint16_t value;
};
typedef struct {
	// Condition variable attribute
    pthread_condattr_t attr;
	// RT-Thread semaphore control block
    rt_semaphore sem;
} pthread_cond_t;
int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
int  pthread_cond_signal ( pthread_cond_t  * cond );
int  pthread_cond_broadcast ( pthread_cond_t  * cond );
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime);

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

bool __builtin_saddl_overflow (long x, long y, long *sum);
bool __builtin_saddll_overflow(long long x, long long y, long long *sum);




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
