// tests/testSystemWideMutex.cpp - test posixfy ipc mutex // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
#include "CTools.h"
#ifdef _WIN32
#	include "Windows/SystemWideMutex.h"
#else
#	include "Posix/SystemWideMutex.h"
#endif
using CSystemWideMutex = Ipc::CSystemWideMutex;
static char g_name[] = "my_lucky_unique_name_for_SystemWideMutex";
constexpr auto now = std::chrono::high_resolution_clock::now;

/*
namespace testSystemWideMutex_ { 

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


TEST(SystemWideMutex_locks, common_environment_immediately_recursive) {
	CSystemWideMutex systemWideMutex1( g_name );
	CSystemWideMutex systemWideMutex2( g_name );
	EXPECT_FALSE( systemWideMutex1.IsError( ) );
	EXPECT_FALSE( systemWideMutex2.IsError( ) );
	EXPECT_TRUE( systemWideMutex1.Lock( 0 ) );
	EXPECT_TRUE( systemWideMutex2.Lock( 0 ) );
}

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
	CSystemWideMutex systemWideMutex( g_name );
	bool success = ( true
			&& !systemWideMutex.IsError( ) 
		);
	EXPECT_TRUE( success );
	if ( !success ) 
		return;

	std::atomic_bool started, stop;
	std::thread thread([&started, &stop, &systemWideMutex] {
			// try take ownership
			EXPECT_TRUE( systemWideMutex.Lock( 0 ) );
			started = true;
			// wait stop flag
			while ( !stop )
				std::this_thread::yield( );
		});
	while ( !started )
		std::this_thread::yield( );
	EXPECT_FALSE( systemWideMutex.Lock( 0 ) );
	stop = true;
	thread.join( );
}

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

TEST(SystemWideMutex_locks, real_world_wait1_1500) {
	CSystemWideMutex systemWideMutex( g_name );
	EXPECT_TRUE( systemWideMutex.Lock( 0 ) );
	std::thread thread([&systemWideMutex] {
			EXPECT_FALSE( systemWideMutex.Lock( 1500 ) );
		});
	thread.join( );
}

TEST(SystemWideMutex_locks, real_world_wait2_1500) {
	CSystemWideMutex systemWideMutex1( g_name );
	CSystemWideMutex systemWideMutex2( g_name );
	EXPECT_TRUE( systemWideMutex2.Lock( 0 ) );
	std::thread thread([&systemWideMutex1] {
			EXPECT_FALSE( systemWideMutex1.Lock( 1500 ) );
		});
	thread.join( );
}

TEST(SystemWideMutex_unlocks, break_LockInfinite) {
	CSystemWideMutex systemWideMutex( g_name );

	EXPECT_TRUE( systemWideMutex.Lock( 0 ) );

	std::atomic_bool started, stoped;
	std::thread thread([&started, &stoped, &systemWideMutex] {
			started = true;
			EXPECT_TRUE( systemWideMutex.LockInfinite( ) );
			stoped = true;
		});
	while ( !started )
		std::this_thread::yield( );
	// waiting less then infinite
	auto next_clock = now( ) + std::chrono::milliseconds{ 100 };
	bool not_atomic_stoped = false;
	while ( now( ) < next_clock ) {
		not_atomic_stoped = stoped;
		if ( not_atomic_stoped )
			break;
		std::this_thread::yield( );
	}
	EXPECT_FALSE( not_atomic_stoped );
	systemWideMutex.Unlock( );

	thread.join( );
}

TEST(SystemWideMutex_tricks, release_mutex) {
	CSystemWideMutex systemWideMutex( g_name );
	systemWideMutex.Unlock( );
#ifdef WIN32
	EXPECT_EQ( ERROR_NOT_OWNER, GetLastError( ) );
#endif // WIN32
	EXPECT_TRUE( systemWideMutex.Lock( 0 ) );
}

} // namespace testSystemWideMutex_ 
//*/
