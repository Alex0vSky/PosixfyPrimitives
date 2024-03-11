// src/Posix/SystemWideMutex.h - ipc mutex facility
#pragma once // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
namespace Ipc {
namespace detail { 
thread_local class ThreadExiter_tls {
	std::function<void()> m_exit_func;

public:
    ThreadExiter_tls() = default;
    ThreadExiter_tls(ThreadExiter_tls const&) = delete; void operator=(ThreadExiter_tls const&) = delete;
    ~ThreadExiter_tls() {
		if ( m_exit_func )
			m_exit_func( );
    }
    void set(std::function<void()> func) {
		m_exit_func = func;
    }   
} g_threadExiter;
} // namespace detail

class CSystemWideMutex {
	WideMutexHandle m_semaphore;
	bool m_open_existing;
	std::thread::id m_creator_tid, m_owner_tid;

public:
	CSystemWideMutex(
		const char *name
		, bool *p_already_exists = nullptr
		, bool open_existing = false
	) :
		m_semaphore( std::string( "\\" ) + name )
		, m_open_existing( open_existing )
		, m_creator_tid( std::this_thread::get_id( ) )
	{
		bool is_exists = false;
		int mode = 0644;
		// allow single lock after creation
		int value = 1;

		sem_t *semaphore = sem_open( m_semaphore.get_name( ), O_RDWR );
		if ( SEM_FAILED == semaphore ) 
			semaphore = sem_open( m_semaphore.get_name( ), O_CREAT | O_EXCL, mode, value );
		else
			is_exists = true;

		if ( open_existing && !is_exists )
			semaphore = SEM_FAILED;
		m_semaphore.set( semaphore );
		
		if ( p_already_exists )
			*p_already_exists = is_exists;
	}

	CSystemWideMutex(const CSystemWideMutex& other) :
		m_semaphore( CTools::CopyHandle( other.m_semaphore ) )
		, m_open_existing( other.m_open_existing )
		, m_creator_tid( other.m_creator_tid )
		, m_owner_tid( other.m_owner_tid )
	{}

	const CSystemWideMutex& operator= (const CSystemWideMutex& other) {
		if ( this != &other ) {
			CTools::CloseAndInvalidateHandle( m_semaphore );
			m_semaphore = CTools::CopyHandle( other.m_semaphore );
			m_open_existing = ( other.m_open_existing );
			m_creator_tid = ( other.m_creator_tid );
			m_owner_tid = ( other.m_owner_tid );
		}
		return *this;
	}

	~CSystemWideMutex() {
		detail::g_threadExiter.set( nullptr );
		CTools::CloseAndInvalidateHandle( m_semaphore );
		if ( !m_open_existing )
			sem_unlink( m_semaphore.get_name( ) );
	}

	bool IsError() const {
		return !m_semaphore;
	}
							
	// returns true if we've got ownership, so Unlock() must be called when ownership is no longer needed
	// it is safe to call Unlock() without corresp. Lock() returned true
	bool Lock(unsigned timeout_milli)  {
		if ( !m_semaphore )
			return false;

		// Implement recursive mutex, prolog
		int sval1;
		if ( -1 == sem_getvalue( m_semaphore, &sval1 ) )
			return false;
		const std::thread::id current_tid = std::this_thread::get_id( );
		if ( current_tid == m_creator_tid && 0 == sval1 ) 
			if ( m_creator_tid == m_owner_tid || std::thread::id{ } == m_owner_tid )
				sem_post( m_semaphore );

		timespec abstime = CTools::MilliToAbsoluteTimespec( timeout_milli );
		// Limitation of `sem_timedwait()` or get 'EINVAL' error. ?`set_normalized_timespec()`
		const unsigned limit = 1'000'000'000;
		if ( abstime.tv_nsec >= limit )
			abstime.tv_nsec = limit - 1;

		bool interupt, success = false; do {
			success = ( !sem_timedwait( m_semaphore, &abstime ) );
			interupt = ( !success && errno == EINTR );
		} while ( interupt ); // If user will use signal handler

		// Implement recursive mutex, epilog
		int sval2;
		if ( -1 == sem_getvalue( m_semaphore, &sval2 ) )
			return false;
		if ( success && 0 == sval2 ) {
			m_owner_tid = current_tid;
			// To reset owner on thread exit
			// TODO(alex): bug, UB if owner destroyed before thread end
			detail::g_threadExiter.set([this] {
					m_owner_tid = std::thread::id{ };
				});
		} else {
			m_owner_tid = std::thread::id{ };
		}

		return success;
	}

	bool LockInfinite() {
		return Lock( INFINITE );
	}
							
	// it is safe to call Unlock() without corresp. Lock() returned true
	void Unlock() {
		if ( !m_semaphore )
			return;
		int sval;
		if ( -1 == sem_getvalue( m_semaphore, &sval ) )
			return;
		if ( 0 == sval )
			sem_post( m_semaphore );
	}

};
} // namespace Ipc
