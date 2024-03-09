// src/Posix/SystemWideMutex.h - ipc mutex facility
#pragma once // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
namespace Ipc {

thread_local static int g_counter = 1;
thread_local struct Foo_tls {
	void save_owner() {
	}
} g_foo;

class CSystemWideMutex {
	sem_t *h_semaphore;
	const std::string m_name;
	bool m_open_existing;
	const std::thread::id m_tid;
	int m_sval;

	// for compare
	const std::thread::id m_empty_tid;
	std::thread::id m_owner_tid;

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

public:
	CSystemWideMutex(const char *name,bool *p_already_exists=NULL,bool open_existing=false) :
		h_semaphore( SEM_FAILED )
		, m_name( std::string( "\\" ) + name )
		, m_open_existing( open_existing )
		, m_tid( std::this_thread::get_id( ) )
	{		
		//m_string_tid = ( ( std::ostringstream( ) << m_tid ).str( ) )

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

	CSystemWideMutex(const CSystemWideMutex& other) = delete;
	//CSystemWideMutex(const CSystemWideMutex& other) {
	//	//h_semaphore = CTools::CopyHandle(other.h_semaphore);
	//}

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
		// ?TODO(alex): cause code-dump
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

		if ( std::this_thread::get_id( ) == m_tid ) {
			int sval;
			sem_getvalue( h_semaphore, &sval );
			printf( "creator thread, BEG sval: %d\n", sval );
			if ( !sval ) {
				if ( m_empty_tid != m_owner_tid ) {
					if ( std::this_thread::get_id( ) == m_owner_tid ) {
						sem_post( h_semaphore );
					}
				}
			}
			sem_getvalue( h_semaphore, &sval );
			printf( "creator thread, END sval: %d\n", sval );
		} else {
			sem_getvalue( h_semaphore, &m_sval ); printf( "other thread, sval: %d\n", m_sval );
		}

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

		sem_getvalue( h_semaphore, &m_sval );
		if ( success && !m_sval ) {
			//g_foo.save_owner( );
			m_owner_tid = std::this_thread::get_id( );
		} else {
			m_owner_tid = m_empty_tid;
		}
		printf( "after wait sval: %d, %s\n", m_sval, ( success ?"true" :"false" ) );
		if ( !success ) perror( "!success" );

		return success;
	}

	bool LockInfinite() {
		return Lock(INFINITE);
	}
							
	// it is safe to call Unlock() without corresp. Lock() returned true
	void Unlock() {
		if ( h_semaphore == SEM_FAILED )
			return;
		sem_getvalue ( h_semaphore, &m_sval );
		if ( !m_sval )
			sem_post( h_semaphore );
	}
	// TODO(alex): broken logic detected, unusable object, handle got from `CreateMutex()/OpenMutex()`

};
} // namespace Ipc
