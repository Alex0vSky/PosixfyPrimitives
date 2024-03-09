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

/*
TEST(SystemWideMutex_create, already_exists) {
	bool already_exists;
	char name[] = "some_name";
	CSystemWideMutex systemWideMutex1( name, &already_exists );
	CSystemWideMutex systemWideMutex2( name, &already_exists );

	EXPECT_TRUE( already_exists );
	EXPECT_FALSE( systemWideMutex2.IsError( ) );
}

TEST(SystemWideMutex_create, ordinary) {
	CSystemWideMutex systemWideMutex( g_name );
	EXPECT_FALSE( systemWideMutex.IsError( ) );
}

TEST(SystemWideMutex_create, freeable) {
	{
		CSystemWideMutex systemWideMutex( g_name );
		EXPECT_FALSE( systemWideMutex.IsError( ) );
	}
	{
		CSystemWideMutex systemWideMutex( g_name );
		EXPECT_FALSE( systemWideMutex.IsError( ) );
	}
}

TEST(SystemWideMutex_create, open_existing_true) {
	bool already_exists;
	char name[] = "some_name";
	CSystemWideMutex systemWideMutex1( name, &already_exists );
	CSystemWideMutex systemWideMutex2( name, &already_exists, true );

	EXPECT_TRUE( already_exists );
	EXPECT_FALSE( systemWideMutex2.IsError( ) );
}

TEST(SystemWideMutex_create, open_existing_false) {
	bool already_exists;
	char name[] = "some_name";
	CSystemWideMutex systemWideMutex1( name, &already_exists );
	CSystemWideMutex systemWideMutex2( g_name, &already_exists, true );

	EXPECT_FALSE( already_exists );
	EXPECT_TRUE( systemWideMutex2.IsError( ) );
}

TEST(SystemWideMutex_locks, simple) {
	CSystemWideMutex systemWideMutex( g_name );
	EXPECT_TRUE( systemWideMutex.Lock( 0 ) );
}

TEST(SystemWideMutex_locks, invalid_object) {
	bool already_exists;
	char name[] = "some_name";
	CSystemWideMutex systemWideMutex1( name, &already_exists );
	CSystemWideMutex systemWideMutex2( g_name, &already_exists, true );

	EXPECT_TRUE( systemWideMutex2.IsError( ) );
	EXPECT_FALSE( systemWideMutex2.Lock( 0 ) );
}
//*/

TEST(SystemWideMutex_locks, common_environment_immediately_recursive) {
	CSystemWideMutex systemWideMutex1( g_name );
	CSystemWideMutex systemWideMutex2( g_name );
	EXPECT_FALSE( systemWideMutex1.IsError( ) );
	EXPECT_FALSE( systemWideMutex2.IsError( ) );
	EXPECT_TRUE( systemWideMutex1.Lock( 0 ) );
	EXPECT_TRUE( systemWideMutex2.Lock( 0 ) );
}

/*
TEST(SystemWideMutex_locks, separate_environment_immediately_not_recursive_by_ref1) {
	CSystemWideMutex systemWideMutex1( g_name );
	CSystemWideMutex systemWideMutex2( g_name );
	bool success = ( true
			&& !systemWideMutex1.IsError( ) 
			&& !systemWideMutex1.IsError( ) 
		);
	EXPECT_TRUE( success );
	if ( !success ) 
		return;

	std::atomic_bool started;
	std::thread thread([&started, &systemWideMutex1] {
			// try take ownership
			EXPECT_TRUE( systemWideMutex1.Lock( 0 ) );
			started = true;
		});
	while ( !started )
		std::this_thread::yield( );
	// wait thread end
	thread.join( );
	EXPECT_TRUE( systemWideMutex2.Lock( 0 ) );
}

TEST(SystemWideMutex_locks, separate_environment_immediately_not_recursive_by_ref2) {
	CSystemWideMutex systemWideMutex1( g_name );
	CSystemWideMutex systemWideMutex2( g_name );
	bool success = ( true
			&& !systemWideMutex1.IsError( ) 
			&& !systemWideMutex1.IsError( ) 
		);
	EXPECT_TRUE( success );
	if ( !success ) 
		return;

	std::atomic_bool start, stop;
	std::thread thread([&start, &stop, &systemWideMutex1] {
			// try take ownership
			EXPECT_TRUE( systemWideMutex1.Lock( 0 ) );
			start = true;
			// wait stop flag
			while ( !stop )
				std::this_thread::yield( );
		});
	while ( !start )
		std::this_thread::yield( );
	EXPECT_FALSE( systemWideMutex2.Lock( 0 ) );
	stop = true;
	thread.join( );
}
//*/

//*
TEST(SystemWideMutex_locks, separate_environment_mix) {
	{
		CSystemWideMutex systemWideMutex( g_name );
		EXPECT_TRUE( systemWideMutex.Lock( 0 ) );
		std::thread thread([&systemWideMutex] {
				EXPECT_FALSE( systemWideMutex.Lock( 0 ) );
			});
		thread.join( );
	}
	{
		CSystemWideMutex systemWideMutex( g_name );
		std::thread thread([&systemWideMutex] {
				EXPECT_TRUE( systemWideMutex.Lock( 0 ) );
			});
		thread.join( );
		EXPECT_TRUE( systemWideMutex.Lock( 0 ) );
	}
}
//*/

//TEST(SystemWideMutex_locks, real_world1_1500) {
//	CSystemWideMutex systemWideMutex1( g_name );
//	EXPECT_TRUE( systemWideMutex1.Lock( 0 ) );
//	std::thread thread([&systemWideMutex1] {
//			EXPECT_FALSE( systemWideMutex1.Lock( 1500 ) );
//		});
//	thread.join( );
//}
//
//TEST(SystemWideMutex_locks, real_world2_1500) {
//	CSystemWideMutex systemWideMutex1( g_name );
//	CSystemWideMutex systemWideMutex2( g_name );
//	EXPECT_TRUE( systemWideMutex2.Lock( 0 ) );
//	std::thread thread([&systemWideMutex1] {
//			EXPECT_FALSE( systemWideMutex1.Lock( 1500 ) );
//		});
//	thread.join( );
//}

/*
TEST(SystemWideMutex_unlocks, break_LockInfinite) {
	CSystemWideMutex systemWideMutex1( g_name );
	CSystemWideMutex systemWideMutex2( g_name );

	EXPECT_TRUE( systemWideMutex2.Lock( 0 ) );
	std::atomic_bool started;
	std::thread thread([&started, &systemWideMutex1] {
			started = true;
			EXPECT_TRUE( systemWideMutex1.LockInfinite( ) );
		});
	while ( !started )
		std::this_thread::yield( );
	std::this_thread::sleep_for( std::chrono::milliseconds{ 500 } );
	systemWideMutex2.Unlock( );

	thread.join( );
}
//*/

} // namespace testSystemWideMutex_ 
