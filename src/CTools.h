// src/CTools.h - common
#pragma once // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)

#if __has_include( "Private.h" )
#	include "Private.h"
#endif

#ifndef INFINITE
#	define INFINITE            0xFFFFFFFF  // Infinite timeout
#endif // INFINITE

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

	static constexpr pthread_cond_t c_invalid = PTHREAD_COND_INITIALIZER;
	typedef pthread_cond_t handle_t;
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
	static pthread_cond_t CopyHandle(pthread_cond_t input) {
		return input;
	}
	static void CloseAndInvalidateHandle(pthread_cond_t &handle) {
		pthread_cond_destroy( &handle );
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
			safe_add( &abstime, &adding ); //abstime.tv_sec += adding.tv_sec; abstime.tv_nsec += adding.tv_nsec;
		}
		return abstime;
	}
#endif // _WIN32
};
