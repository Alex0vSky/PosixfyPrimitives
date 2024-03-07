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

#if ( defined( _WIN32 ) )
// System
#define NOMINMAX
#ifdef _DEBUG
#	include <crtdbg.h>
#endif
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <intrin.h>
#endif // ( defined( _WIN32 ) )

#include <stdlib.h>

//// boost
//#include <boost/Xxx.hpp>
#pragma warning( pop )
