// tests/testSystemWideMutex.cpp - test posixfy ipc mutex // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
#include "CTools.h"
#ifdef _WIN32
#	include "Windows/SystemWideMutex.h"
#else
#	include "Posix/SystemWideMutex.h"
#endif
using CSystemWideMutex = Ipc::CSystemWideMutex;
static char g_name[] = "my_lucky_unique_name_for_SystemWideMutex";
static char g_name2[] = "my_lucky_unique_name_for_SystemWideMutex2";
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

TEST(SystemWideMutex_locks, separate_environment_immediately_norecursive_by_ref1) {
	CSystemWideMutex systemWideMutex1( g_name );
	CSystemWideMutex systemWideMutex2( g_name );
	bool success = ( true
			&& !systemWideMutex1.IsError( ) 
			&& !systemWideMutex1.IsError( ) 
		);
	EXPECT_TRUE( success );
	if ( !success ) 
		GTEST_SKIP( );

	std::atomic_bool started;
	started = false;
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

TEST(SystemWideMutex_locks, separate_environment_immediately_norecursive_by_ref2) {
	CSystemWideMutex systemWideMutex( g_name );
	EXPECT_FALSE( systemWideMutex.IsError( ) );
	if ( systemWideMutex.IsError( ) )
		GTEST_SKIP( );

	std::atomic_bool started, stop;
	started = stop = false;
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
	EXPECT_FALSE( systemWideMutex.IsError( ) );
	if ( systemWideMutex.IsError( ) )
		GTEST_SKIP( );

	EXPECT_TRUE( systemWideMutex.Lock( 0 ) );

	std::atomic_bool started, stoped;
	started = stoped = false;
	std::thread thread([&started, &stoped, &systemWideMutex] {
			started = true;
			EXPECT_TRUE( systemWideMutex.LockInfinite( ) );
			stoped = true;
		});
	while ( !started )
		std::this_thread::yield( );
	// waiting less then infinite
	auto next_clock = now( ) + std::chrono::milliseconds{ 100 };
	bool noatomic_stoped = false;
	while ( now( ) < next_clock ) {
		noatomic_stoped = stoped;
		if ( noatomic_stoped )
			break;
		std::this_thread::yield( );
	}
	EXPECT_FALSE( noatomic_stoped );
	systemWideMutex.Unlock( );

	thread.join( );
}

TEST(SystemWideMutex_tricks, dry_unlock) {
	CSystemWideMutex systemWideMutex( g_name );
	systemWideMutex.Unlock( );
#ifdef WIN32
	EXPECT_EQ( ERROR_NOT_OWNER, GetLastError( ) );
#endif // WIN32
	EXPECT_TRUE( systemWideMutex.Lock( 0 ) );
}

TEST(SystemWideMutex_tricks, multi_lock_single_unlock) {
	CSystemWideMutex systemWideMutex( g_name );
	EXPECT_TRUE( systemWideMutex.Lock( 0 ) );
	EXPECT_TRUE( systemWideMutex.Lock( 0 ) );
	systemWideMutex.Unlock( );
	EXPECT_TRUE( systemWideMutex.Lock( 0 ) );
}

TEST(SystemWideMutex_copy_ctor, separate_environment) {
	auto systemWideMutex1 = std::make_unique< CSystemWideMutex >( g_name );
	CSystemWideMutex systemWideMutex2( *systemWideMutex1 );
	bool success = ( true
			&& !systemWideMutex1 ->IsError( ) 
			&& !systemWideMutex2.IsError( ) 
		);
	EXPECT_TRUE( success );
	if ( !success ) 
		GTEST_SKIP( );
	// to ensurance that a first object is not exists 
	systemWideMutex1.reset( );

	std::atomic_bool started, stop;
	started = stop = false;
	std::thread thread([&started, &stop, &systemWideMutex2] {
			// try take ownership
			EXPECT_TRUE( systemWideMutex2.Lock( 0 ) );
			started = true;
			// wait stop flag
			while ( !stop )
				std::this_thread::yield( );
		});
	while ( !started )
		std::this_thread::yield( );
	EXPECT_FALSE( systemWideMutex2.Lock( 0 ) );
	stop = true;
	thread.join( );
}

TEST(SystemWideMutex_assignment, separate_environment) {
	auto systemWideMutex1 = std::make_unique< CSystemWideMutex >( g_name );
	CSystemWideMutex systemWideMutex2( g_name2 );
	systemWideMutex2 = *systemWideMutex1;
	bool success = ( true 
			&& !systemWideMutex1 ->IsError( ) 
			&& !systemWideMutex2.IsError( ) 
		);
	EXPECT_TRUE( success );
	if ( !success ) 
		GTEST_SKIP( );
	// to ensurance that a first object is not exists 
	systemWideMutex1.reset( );

	std::atomic_bool started, stop;
	started = stop = false;
	std::thread thread([&started, &stop, &systemWideMutex2] {
			// try take ownership
			EXPECT_TRUE( systemWideMutex2.Lock( 0 ) );
			started = true;
			// wait stop flag
			while ( !stop )
				std::this_thread::yield( );
		});
	while ( !started )
		std::this_thread::yield( );
	EXPECT_FALSE( systemWideMutex2.Lock( 0 ) );
	stop = true;
	thread.join( );
}

TEST(SystemWideMutex_bug_in_my_posix_impl, owner_unlock_on_thread_end) {
	auto systemWideMutex1 = std::make_unique< CSystemWideMutex >( g_name );
	CSystemWideMutex systemWideMutex2( g_name2 );
	systemWideMutex2 = *systemWideMutex1;
	bool success = ( true 
			&& !systemWideMutex1 ->IsError( ) 
			&& !systemWideMutex2.IsError( ) 
		);
	EXPECT_TRUE( success );
	if ( !success ) 
		GTEST_SKIP( );

	std::atomic_bool started, stop;
	started = stop = false;
	std::thread thread([&started, &stop, &systemWideMutex1] {
			// try take ownership
			EXPECT_TRUE( systemWideMutex1 ->Lock( 0 ) );
			systemWideMutex1.reset( );
			started = true;
			// wait stop flag
			while ( !stop )
				std::this_thread::yield( );
		});
	while ( !started )
		std::this_thread::yield( );
#ifdef _WIN32
	EXPECT_FALSE( systemWideMutex2.Lock( 0 ) );
#else
	EXPECT_TRUE( systemWideMutex2.Lock( 0 ) );
#endif
	stop = true;
	thread.join( );
}

} // namespace testSystemWideMutex_ 
//*/
