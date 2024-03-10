// src/Posix/SystemWideMutex.h - ipc mutex facility
#pragma once // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
namespace Ipc {

namespace detail { 
thread_local class ThreadExiter {
	std::function<void()> m_exit_func;

public:
    ThreadExiter() = default;
    ThreadExiter(ThreadExiter const&) = delete; void operator=(ThreadExiter const&) = delete;
    ~ThreadExiter() {
		if ( m_exit_func )
			m_exit_func( );
    }
    void set(std::function<void()> func) {
		m_exit_func = func;
    }   
} g_threadExiter;
} // namespace detail

class CSystemWideMutex {
	sem_t *h_semaphore;
	const std::string m_name;
	bool m_open_existing;
	const std::thread::id m_creator_tid;

	std::thread::id m_owner_tid;
	// for tidy compare
	const std::thread::id m_empty_tid;

	// @insp https://stackoverflow.com/questions/15024623/convert-milliseconds-to-timespec-for-gnu-port
	static void ms2ts(timespec *ts, unsigned long milli) {
		ts ->tv_sec = milli / 1000;
		ts ->tv_nsec = (milli % 1000) * 1000000;
	}
	// To avoid Integer overflow: abstime.tv_sec += adding.tv_sec; abstime.tv_nsec += adding.tv_nsec;
	static void safe_add(timespec *accum, timespec *src) {
		long sum_tv_sec; // decltype( timespec::tv_sec ) 
		if ( __builtin_saddl_overflow( accum ->tv_sec, src ->tv_sec, &sum_tv_sec ) ) {
			accum ->tv_sec = std::numeric_limits< long >::max( );
		} else {
			accum ->tv_sec = sum_tv_sec;
		}
		long sum_tv_nsec;
		if ( __builtin_saddl_overflow( accum ->tv_nsec, src ->tv_nsec, &sum_tv_nsec ) ) {
			accum ->tv_nsec = std::numeric_limits< long >::max( );
		} else {
			accum ->tv_nsec = sum_tv_nsec;
		}
	}

	int getvalue() const {
		int sval;
		// ignore error
		sem_getvalue( h_semaphore, &sval );
		return sval;
	}

public:
	CSystemWideMutex(const char *name,bool *p_already_exists=NULL,bool open_existing=false) :
		h_semaphore( SEM_FAILED )
		, m_name( std::string( "\\" ) + name )
		, m_open_existing( open_existing )
		, m_creator_tid( std::this_thread::get_id( ) )
	{		
		//m_string_tid = ( ( std::ostringstream( ) << m_creator_tid ).str( ) )

		bool is_exists = false;
		//int mode = 0644;
		int mode = 0777;
		// allow single lock after creation
		int value = 1;

		h_semaphore = sem_open( m_name.c_str( ), O_RDWR );
		if ( SEM_FAILED == h_semaphore ) {
			h_semaphore = sem_open( m_name.c_str( ), O_CREAT | O_EXCL, mode, value );
		} else {
			is_exists = true;
		}
		if ( open_existing && !is_exists )
			h_semaphore = SEM_FAILED;

		if ( p_already_exists )
			*p_already_exists = is_exists;
	}

	//CSystemWideMutex(const CSystemWideMutex& other) = delete;
	CSystemWideMutex(const CSystemWideMutex& other) :
		h_semaphore( SEM_FAILED )
		, m_name( other.m_name )
		, m_open_existing( other.m_open_existing )
		, m_creator_tid( other.m_creator_tid )
		, m_owner_tid( other.m_owner_tid )
	{}

	const CSystemWideMutex& operator = (const CSystemWideMutex& other) = delete;
	//const CSystemWideMutex& operator = (const CSystemWideMutex& other) {
	//	if ( this != &other )
	//	{
	//		//CTools::CloseAndInvalidateHandle(h_semaphore);
	//		//h_semaphore = CTools::CopyHandle(other.h_semaphore);
	//	}
	//	return *this;
	//}

	~CSystemWideMutex() {
		if ( h_semaphore == SEM_FAILED )
			return;
		// TODO(alex): via iface `CTools::CloseAndInvalidateHandle(h_semaphore);`
		sem_close( h_semaphore ), h_semaphore = SEM_FAILED;
		if ( !m_open_existing )
			sem_unlink( m_name.c_str( ) );
	}

	bool IsError() const {
		return h_semaphore == SEM_FAILED;
	}
							
	// returns true if we've got ownership, so Unlock() must be called when ownership is no longer needed
	// it is safe to call Unlock() without corresp. Lock() returned true
	bool Lock(unsigned timeout_milli)  {
		if ( h_semaphore == SEM_FAILED )
			return false;

		// implement recursive mutex, prolog
		int sval1;
		if ( -1 == sem_getvalue( h_semaphore, &sval1 ) )
			return false;
		const std::thread::id current_tid = std::this_thread::get_id( );
		if ( current_tid == m_creator_tid ) 
			if ( !sval1 ) 
				if ( m_creator_tid == m_owner_tid || m_empty_tid == m_owner_tid )
					sem_post( h_semaphore );

		// TODO(alex): to separate
		timespec abstime = { };
		if ( INFINITE == timeout_milli ) {
			abstime.tv_sec = std::numeric_limits< decltype( abstime.tv_sec ) >::max( );
			abstime.tv_nsec = std::numeric_limits< decltype( abstime.tv_nsec ) >::max( );
		} else {
			abstime.tv_sec = time( nullptr );
			timespec adding = { }; 
			ms2ts( &adding, timeout_milli );
			safe_add( &abstime, &adding ); //abstime.tv_sec += adding.tv_sec; abstime.tv_nsec += adding.tv_nsec;
		}
		// Limitation of `sem_timedwait()` or get 'EINVAL' error. ?`set_normalized_timespec()`
		const unsigned limit = 1'000'000'000;
		if ( abstime.tv_nsec >= limit )
			abstime.tv_nsec = limit - 1;

		// If user will use signal handler
		bool interupt, success = false;
		do {
			success = ( !sem_timedwait( h_semaphore, &abstime ) );
			interupt = ( !success && errno == EINTR );
		} while ( interupt );

		// implement recursive mutex, epilog
		int sval2;
		if ( -1 == sem_getvalue( h_semaphore, &sval2 ) )
			return false;
		if ( success && !sval2 ) {
			m_owner_tid = current_tid;
			//detail::g_threadExiter.set([this] {
			//		m_owner_tid = m_empty_tid;
			//	});
		} else {
			m_owner_tid = m_empty_tid;
		}

		return success;
	}

	bool LockInfinite() {
		return Lock(INFINITE);
	}
							
	// it is safe to call Unlock() without corresp. Lock() returned true
	void Unlock() {
		if ( h_semaphore == SEM_FAILED )
			return;
		int sval;
		if ( -1 == sem_getvalue( h_semaphore, &sval ) )
			return;
		if ( !sval )
			sem_post( h_semaphore );
	}
	// TODO(alex): broken logic detected, unusable object, handle got from `CreateMutex()/OpenMutex()`

};
} // namespace Ipc
