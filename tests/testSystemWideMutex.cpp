﻿// tests/testSystemWideMutex.cpp - test posixfy ipc mutex // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
#include "CTools.h"
#ifdef _WIN32
#	include "Windows/SystemWideMutex.h"
#else
#	include "Posix/SystemWideMutex.h"
#endif
using CSystemWideMutex = Ipc::CSystemWideMutex;
static const unsigned c_size = 4096;
static char g_name[] = "my_lucky_unique_name";

namespace testSharedMemory_ { 

TEST(SystemWideMutex_create, already_exists) {
	bool already_exists;
	char name[] = "some_name";
	CSystemWideMutex systemWideMutex1 = CSystemWideMutex( name, &already_exists );
	CSystemWideMutex systemWideMutex2 = CSystemWideMutex( name, &already_exists );

	EXPECT_TRUE( already_exists );
	EXPECT_FALSE( systemWideMutex2.IsError( ) );
}

} // namespace testSharedMemory_ 
