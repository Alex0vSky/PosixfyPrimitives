﻿// tests/testEvent.cpp - test posixfy event // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
#include "CTools.h"
#ifdef _WIN32
#	include "Windows/Event.h"
#else
#	include "Posix/Event.h"
#endif
using CEvent = IndependentProcess::CEvent;

class set_and_check : public ::testing::TestWithParam< std::tuple< bool, bool > > {};
TEST_P(set_and_check,) {
	CEvent event( std::get<0>( GetParam( ) ), std::get<1>( GetParam( ) ) );
	event.Set( );
	EXPECT_TRUE( event.IsSet( ) );
}
INSTANTIATE_TEST_SUITE_P(
		event_set
		, set_and_check
		, ::testing::Combine( 
				::testing::Values( true, false )
				, ::testing::Values( true, false )
			)
	);

class reset_and_check : public ::testing::TestWithParam< std::tuple< bool, bool > > {};
TEST_P(reset_and_check,) {
	CEvent event( std::get<0>( GetParam( ) ), std::get<1>( GetParam( ) ) );
	event.Reset( );
	EXPECT_FALSE( event.IsSet( ) );
}
INSTANTIATE_TEST_SUITE_P(
		event_reset
		, reset_and_check
		, ::testing::Combine( 
				::testing::Values( true, false )
				, ::testing::Values( true, false )
			)
	);

TEST(event_set, wait0) {	
	CEvent event( false, false );
	event.Set( );
	EXPECT_TRUE( event.Wait( 0 ) );
}

TEST(event_set, waitInfinite) { 
	CEvent event( false, false );
	event.Set( );
	EXPECT_TRUE( event.WaitInfinite( ) );
}

TEST(event_ctor_behavior, manually_reset_forever_setted) { 
	CEvent event( true, false );
	event.Set( );
	event.IsSet( );
	EXPECT_TRUE( event.IsSet( ) );
}

TEST(event_ctor_behavior, manually_reset_resetable) { 
	CEvent event( true, false );
	event.Set( );
	event.Reset( );
	EXPECT_FALSE( event.IsSet( ) );
}

TEST(event_ctor_behavior, reset_after_wait1) { 
	CEvent event( false, false );
	event.Set( );
	event.IsSet( );
	EXPECT_FALSE( event.IsSet( ) );
}

TEST(event_ctor_behavior, reset_after_wait2) { 
	CEvent event( false, false );
	event.Set( );
	event.Wait( 0 );
	EXPECT_FALSE( event.IsSet( ) );
}

TEST(event_ctor_behavior, initial_state_on) { 
	CEvent event( false, true );
	EXPECT_TRUE( event.IsSet( ) );
}

TEST(event_ctor_behavior, initial_state_off) { 
	CEvent event( false, false );
	EXPECT_FALSE( event.IsSet( ) );
}
