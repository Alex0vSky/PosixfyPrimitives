// stdafx.h - pch

#pragma warning( push )
#pragma warning( disable: 4389 )

#ifdef __clang__
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wmicrosoft-include"
#endif // __clang__

// for `strnlen_s()`
#define __STDC_WANT_LIB_EXT1__ 1

#ifdef _WIN32
// Got from NuGet
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#else // _WIN32
#include "include/gtest/gtest.h"
#include "include/gmock/gmock.h"
#endif // _WIN32

#ifdef __clang__
#	pragma clang diagnostic pop
#endif // __clang__

// System
#ifdef _WIN32
#	define NOMINMAX
#	ifdef _DEBUG
#		include <crtdbg.h>
#	endif
#	define WIN32_LEAN_AND_MEAN
#	include <Windows.h>
#	include <intrin.h>
#	include <sddl.h>
#else
#	include <pthread.h>
#	include <sys/mman.h>
#	include <unistd.h>
#	include <errno.h>
#	include <sys/stat.h>
#	include <fcntl.h>
#	include <semaphore.h>
#	include <spawn.h>
#	include <signal.h>
#	include <string.h>
#endif // _WIN32

#include <stdlib.h>
#include <iostream>
#include <thread>

//// boost
//#include <boost/Xxx.hpp>
#pragma warning( pop )
