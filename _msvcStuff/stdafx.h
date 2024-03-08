// stdafx.h - pch

#ifdef __clang__
#	pragma clang diagnostic push
#	pragma clang diagnostic ignored "-Wmicrosoft-include"
#endif // __clang__

#include "include/gtest/gtest.h"
#include "include/gmock/gmock.h"

#ifdef __clang__
#	pragma clang diagnostic pop
#endif // __clang__

#pragma warning( push )
//#pragma warning( disable: 5039 )

// System
#ifdef _WIN32
#	define NOMINMAX
#	ifdef _DEBUG
#		include <crtdbg.h>
#	endif
#	define WIN32_LEAN_AND_MEAN
#	include <Windows.h>
#	include <intrin.h>
#else
#	include <pthread.h>
#	include <sys/mman.h>
#	include <unistd.h>
#	include <errno.h>
#	include <sys/stat.h>
#	include <fcntl.h>
#endif // _WIN32

#include <stdlib.h>
#include <iostream>
#include <thread>

//// boost
//#include <boost/Xxx.hpp>
#pragma warning( pop )
