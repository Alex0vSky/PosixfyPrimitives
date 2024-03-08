// src/CTools.h - common
#pragma once // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)

#undef A0S_INDEVELOP

#ifdef A0S_INDEVELOP
#undef _WIN32
#define PTHREAD_COND_INITIALIZER {{0}}
struct priority_queue_t { int foo; };
struct pthread_condattr_t { int bar; };
struct rt_ipc_object { int foo; };
struct pthread_mutex_t { int foo; };
struct pthread_mutexattr_t { int foo; };
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

// !!! from my imagination
#define PROT_EXEC 1 // Page can be executed.
#define PROT_NONE 2 // Page cannot be accessed.
#define PROT_READ 3 // Page can be read.
#define PROT_WRITE 5 // Page can be written.
#define MAP_FIXED 1 // Interpret addr exactly.
#define MAP_PRIVATE 2 // Changes are private.
#define MAP_SHARED 3 // Share changes.
#define O_CREAT 1
#define O_RDWR 2
#define O_EXCL 3

#define S_IRUSR 1
#define S_IWUSR 2
#define S_IRGRP 3
#define S_IWGRP 6
#define S_IROTH 7 
#define S_IWOTH 9

#define MAP_FAILED ((void *) -1)
//#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); } while (0)
typedef int mode_t;

void *mmap(void *, size_t, int, int, int, off_t);
int munmap(void *addr, size_t len);
int shm_open(const char *, int, mode_t);
int shm_unlink(const char *);
int ftruncate(int fildes, off_t length);

#endif // A0S_INDEVELOP



#ifndef INFINITE
#	define INFINITE            0xFFFFFFFF  // Infinite timeout
#endif // INFINITE

struct CTools {
#ifdef _WIN32
	template <HANDLE invalid_value>
	struct C{
		operator HANDLE() const {
			return INVALID_HANDLE_VALUE;
		}
	};
	static constexpr C<INVALID_HANDLE_VALUE> c_invalid = { };
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
		out = input;
#endif
		return out;
	}
	static void CloseAndInvalidateHandle(handle_t &handle) {
#ifdef _WIN32
		if ( c_invalid == handle || !handle )
			return;
		CloseHandle( handle );
// /usr/bin/ld: CMakeFiles/PosixfyPrimitives.dir/home/runner/work/PosixfyPrimitives/PosixfyPrimitives/tests/testEvent.cpp.o: in function `CTools::CloseAndInvalidateHandle(pthread_cond_t&)':
// testEvent.cpp:(.text._ZN6CTools24CloseAndInvalidateHandleER14pthread_cond_t[_ZN6CTools24CloseAndInvalidateHandleER14pthread_cond_t]+0x1c): undefined reference to `CTools::c_invalid'
		handle = CTools::c_invalid;
#else
		pthread_cond_destroy( &handle );
#endif
	}
};
