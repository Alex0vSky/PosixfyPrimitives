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
	std::string m_name;
	WideMutexHandle h_semaphore2;
	bool m_open_existing;
	std::thread::id m_creator_tid;
	std::thread::id m_owner_tid;
	// for tidy compare
	const std::thread::id m_empty_tid;

public:
	CSystemWideMutex(const char *name,bool *p_already_exists=NULL,bool open_existing=false) :
		m_name( std::string( "\\" ) + name )
		, h_semaphore2( m_name )
		, m_open_existing( open_existing )
		, m_creator_tid( std::this_thread::get_id( ) )
	{		
		bool is_exists = false;
		//int mode = 0644;
		int mode = 0777;
		// allow single lock after creation
		int value = 1;

		sem_t *h_semaphore = sem_open( m_name.c_str( ), O_RDWR );
		if ( SEM_FAILED == h_semaphore ) {
			h_semaphore = sem_open( m_name.c_str( ), O_CREAT | O_EXCL, mode, value );
		} else {
			is_exists = true;
		}
		if ( open_existing && !is_exists )
			h_semaphore = SEM_FAILED;
		h_semaphore2 = h_semaphore;
		
		if ( p_already_exists )
			*p_already_exists = is_exists;
	}

	CSystemWideMutex(const CSystemWideMutex& other) :
		// +-TODO(alex): via iface `CTools::CopyHandle(other);`
		h_semaphore2( CTools::CopyHandle( other.h_semaphore2 ) )
		, m_name( other.m_name )
		, m_open_existing( other.m_open_existing )
		, m_creator_tid( other.m_creator_tid )
		, m_owner_tid( other.m_owner_tid )
	{}

	const CSystemWideMutex& operator= (const CSystemWideMutex& other) {
		if ( this != &other ) {
			// +-TODO(alex): via iface `CTools::CloseAndInvalidateHandle();`
			CTools::CloseAndInvalidateHandle( h_semaphore2 );
			//if ( !m_open_existing )
			//	sem_unlink( m_name.c_str( ) );
			// +-TODO(alex): via iface `CTools::CopyHandle(other);`
			h_semaphore2 = CTools::CopyHandle( other.h_semaphore2 );
			m_name = ( other.m_name );
			m_open_existing = ( other.m_open_existing );
			m_creator_tid = ( other.m_creator_tid );
			m_owner_tid = ( other.m_owner_tid );
		}
		return *this;
	}

	~CSystemWideMutex() {
		//if ( h _semaphore == SEM_FAILED )
		//	return;
		detail::g_threadExiter.set( nullptr );
		// +-TODO(alex): via iface `CTools::CloseAndInvalidateHandle();`
		CTools::CloseAndInvalidateHandle( h_semaphore2 );
		if ( !m_open_existing )
			sem_unlink( m_name.c_str( ) );
	}

	bool IsError() const {
		return !h_semaphore2;
	}
							
	// returns true if we've got ownership, so Unlock() must be called when ownership is no longer needed
	// it is safe to call Unlock() without corresp. Lock() returned true
	bool Lock(unsigned timeout_milli)  {
		//if ( h _semaphore == SEM_FAILED )
		if ( !h_semaphore2 )
			return false;

		// implement recursive mutex, prolog
		int sval1;
		if ( -1 == sem_getvalue( h_semaphore2, &sval1 ) )
			return false;
		const std::thread::id current_tid = std::this_thread::get_id( );
		if ( current_tid == m_creator_tid ) 
			if ( !sval1 ) 
				if ( m_creator_tid == m_owner_tid || m_empty_tid == m_owner_tid )
					sem_post( h_semaphore2 );

		timespec abstime = CTools::MilliToAbsoluteTimespec( timeout_milli );
		// Limitation of `sem_timedwait()` or get 'EINVAL' error. ?`set_normalized_timespec()`
		const unsigned limit = 1'000'000'000;
		if ( abstime.tv_nsec >= limit )
			abstime.tv_nsec = limit - 1;

		bool interupt, success = false; do {
			success = ( !sem_timedwait( h_semaphore2, &abstime ) );
			interupt = ( !success && errno == EINTR );
		} while ( interupt ); // If user will use signal handler

		// implement recursive mutex, epilog
		int sval2;
		if ( -1 == sem_getvalue( h_semaphore2, &sval2 ) )
			return false;
		if ( success && !sval2 ) {
			m_owner_tid = current_tid;
			// To reset owner on thread exit
			// TODO(alex): bug, UB if owner destroyed before thread end
			detail::g_threadExiter.set([this] {
					m_owner_tid = m_empty_tid;
				});
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
		//if ( h _semaphore == SEM_FAILED )
		if ( !h_semaphore2 )
			return;
		int sval;
		if ( -1 == sem_getvalue( h_semaphore2, &sval ) )
			return;
		if ( !sval )
			sem_post( h_semaphore2 );
	}

};
} // namespace Ipc
