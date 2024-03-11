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
};
