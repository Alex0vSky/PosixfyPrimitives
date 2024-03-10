// src/CTools.h - common
#pragma once // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)

#if __has_include( "Private.h" )
#	include "Private.h"
#endif

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
