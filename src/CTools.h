// src/CTools.h - common
#pragma once // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)

#if __has_include( "Private.h" )
#	include "Private.h"
#endif

#ifndef INFINITE
#	define INFINITE            0xFFFFFFFF  // Infinite timeout
#endif // INFINITE

#ifndef _WIN32
// opaque type wrapper @insp SO/how-can-i-test-equality-of-two-pthread-mutexes
// POSIX documentation states that: "There are no defined comparison or assignment operators for the types pthread_attr_t, pthread_cond_t, pthread_condattr_t, pthread_mutex_t, pthread_mutexattr_t, pthread_rwlock_t and pthread_rwlockattr_t."
class EventHandle {
	bool m_valid;
	pthread_cond_t m_handle;

public:
	EventHandle() : m_valid( true ) {
		pthread_cond_init( &m_handle, nullptr );
	}
	void CloseAndInvalidateHandle() {
		if ( !m_valid )
			return;
		pthread_cond_destroy( &m_handle );
		m_valid = false;
	}
	operator pthread_cond_t *() {
		return &m_handle;
	}
	explicit operator bool() const {
		return m_valid;
	}
};

// forward decl
namespace Ipc{ class CSystemWideMutex; } // namespace Ipc

class WideMutexHandle {
	bool m_valid;
	std::string m_name;
	sem_t *m_handle;

	friend class ::Ipc::CSystemWideMutex;
	void set(sem_t *semaphore) {
		m_valid = ( semaphore != SEM_FAILED );
		m_handle = ( semaphore );
	}

public:
	explicit WideMutexHandle(std::string const& name) : 
		m_valid( false ) 
		, m_name( name ) 
		, m_handle( SEM_FAILED )
	{}
	WideMutexHandle(WideMutexHandle const& rhs) : 
		m_valid( rhs.m_valid ) 
		, m_name( rhs.m_name ) 
	{
		set( sem_open( m_name.c_str( ), O_RDWR ) );
	}
	void CloseAndInvalidateHandle() {
		if ( !m_valid )
			return;
		sem_close( m_handle ), m_handle = SEM_FAILED;
		m_valid = false;
	}
	const char *get_name() const {
		return m_name.c_str( );
	}
	operator sem_t *() {
		return m_handle;
	}
	explicit operator bool() const {
		return m_valid;
	}
};
#endif // _WIN32

class CTools {
#ifdef _WIN32
	template <HANDLE invalid_value>
	struct static_constexpr {
		operator HANDLE() const {
			return INVALID_HANDLE_VALUE;
		}
	};
	static constexpr static_constexpr<INVALID_HANDLE_VALUE> c_invalid = { };

public:
	static HANDLE CopyHandle(HANDLE input) {
		HANDLE out = c_invalid;
		HANDLE process = GetCurrentProcess( );
		// The interface clearly indicates to intraprocess duplication
		::DuplicateHandle( process, 
				input, 
				process,
				&out, 
				0, 
				FALSE, 
				DUPLICATE_SAME_ACCESS
			);
		return out;
	}
	static void CloseAndInvalidateHandle(HANDLE &handle) {
		if ( c_invalid != handle && 0 != handle )
			CloseHandle( handle );
		handle = c_invalid;
	}
#else // _WIN32
	// @insp SO/convert-milliseconds-to-timespec-for-gnu-port
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
	static EventHandle CopyHandle(EventHandle input) {
		return input;
	}
	static WideMutexHandle CopyHandle(WideMutexHandle const& input) {
		return WideMutexHandle( input );
	}
	static void CloseAndInvalidateHandle(EventHandle &handle) {
		handle.CloseAndInvalidateHandle( );
	}
	static void CloseAndInvalidateHandle(WideMutexHandle &handle) {
		handle.CloseAndInvalidateHandle( );
	}

	static timespec MilliToAbsoluteTimespec(unsigned milli=0) {
		timespec abstime = { };
		if ( INFINITE == milli ) {
			abstime.tv_sec = std::numeric_limits< decltype( abstime.tv_sec ) >::max( );
			abstime.tv_nsec = std::numeric_limits< decltype( abstime.tv_nsec ) >::max( );
		} else {
			abstime.tv_sec = time( nullptr ); // clock_gettime( CLOCK_REALTIME, &abstime );
			timespec adding = { }; 
			ms2ts( &adding, milli );
			safe_add( &abstime, &adding );
		}
		return abstime;
	}
#endif // _WIN32
};
