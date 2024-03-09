// tests/testSystemWideMutex.cpp - test posixfy ipc mutex // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
#include "CTools.h"
#ifdef _WIN32
#	include "Windows/SystemWideMutex.h"
#else
#	include "Posix/SystemWideMutex.h"
#endif
using CSystemWideMutex = Ipc::CSystemWideMutex;
static const unsigned c_size = 4096;
static char g_name[] = "my_lucky_unique_name_for_SystemWideMutex";

namespace testSystemWideMutex_ { 

TEST(SystemWideMutex_create, already_exists) {
	bool already_exists;
	char name[] = "some_name";
	CSystemWideMutex systemWideMutex1 = CSystemWideMutex( name, &already_exists );
	CSystemWideMutex systemWideMutex2 = CSystemWideMutex( name, &already_exists );

	EXPECT_TRUE( already_exists );
	EXPECT_FALSE( systemWideMutex2.IsError( ) );
}

TEST(SystemWideMutex_create, ordinary) {
	CSystemWideMutex systemWideMutex = CSystemWideMutex( g_name );
	EXPECT_FALSE( systemWideMutex.IsError( ) );
}

TEST(SystemWideMutex_create, freeable) {
	{
		CSystemWideMutex systemWideMutex = CSystemWideMutex( g_name );
		EXPECT_FALSE( systemWideMutex.IsError( ) );
	}
	{
		CSystemWideMutex systemWideMutex = CSystemWideMutex( g_name );
		EXPECT_FALSE( systemWideMutex.IsError( ) );
	}
}

TEST(SystemWideMutex_create, open_existing_true) {
	bool already_exists;
	char name[] = "some_name";
	CSystemWideMutex systemWideMutex1 = CSystemWideMutex( name, &already_exists );
	CSystemWideMutex systemWideMutex2 = CSystemWideMutex( name, &already_exists, true );

	EXPECT_TRUE( already_exists );
	EXPECT_FALSE( systemWideMutex2.IsError( ) );
}

TEST(SystemWideMutex_create, open_existing_false) {
	bool already_exists;
	char name[] = "some_name";
	CSystemWideMutex systemWideMutex1 = CSystemWideMutex( name, &already_exists );
	CSystemWideMutex systemWideMutex2 = CSystemWideMutex( g_name, &already_exists, true );

	EXPECT_FALSE( already_exists );
	EXPECT_TRUE( systemWideMutex2.IsError( ) );
}
} // namespace testSystemWideMutex_ 

