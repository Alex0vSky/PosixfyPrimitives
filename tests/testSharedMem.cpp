// tests/testSharedMem.cpp - test posixfy ipc shared memory // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
#include "CTools.h"
#ifdef _WIN32
#	include "Windows/SharedMem.h"
#else
#	include "Posix/SharedMem.h"
#endif
using CSharedMem = Ipc::CSharedMem;
static const unsigned c_size = 4096;
static char g_name[] = "my_lucky_unique_name";

namespace testSharedMemory_ { 

TEST(SharedMemory_create, already_exists) {
	bool already_exists;
	char name[] = "some_name";
	CSharedMem *sharedMem1 = CSharedMem::Create( name, c_size, &already_exists );
	CSharedMem *sharedMem2 = CSharedMem::Create( name, c_size, &already_exists );

	EXPECT_TRUE( already_exists );
	EXPECT_FALSE( sharedMem2 ->IsError( ) );

	CSharedMem::Free( sharedMem2 );
	CSharedMem::Free( sharedMem1 );
}

TEST(SharedMemory_create, ordinary) {
	CSharedMem *sharedMem = CSharedMem::Create( g_name, c_size );
	EXPECT_FALSE( sharedMem ->IsError( ) );
	CSharedMem::Free( sharedMem );
}

TEST(SharedMemory_create, freeable) {
	CSharedMem *sharedMem = nullptr;
	sharedMem = CSharedMem::Create( g_name, c_size );
	EXPECT_FALSE( sharedMem ->IsError( ) );
	CSharedMem::Free( sharedMem );

	bool already_exists;
	sharedMem = CSharedMem::Create( g_name, c_size, &already_exists );
	EXPECT_FALSE( already_exists );
	EXPECT_FALSE( sharedMem ->IsError( ) );
	CSharedMem::Free( sharedMem );
}

TEST(SharedMemory_using, create_write_open_read) {
	CSharedMem *sharedMemWriter = CSharedMem::Create( g_name, c_size );
	CSharedMem *sharedMemReader = CSharedMem::Open( g_name );

	EXPECT_FALSE( sharedMemWriter ->IsError( ) );
	EXPECT_FALSE( sharedMemReader ->IsError( ) );

	if ( true
		&& !sharedMemWriter ->IsError( ) 
		&& !sharedMemReader ->IsError( ) 
	) {
		auto writer = reinterpret_cast<char *>( sharedMemWriter ->GetMemPtr( ) );
		for ( int i = 0; i < c_size; ++i ) 
			writer[ i ] = static_cast< char >( i );

		auto reader = reinterpret_cast<const char *>( sharedMemReader ->GetMemPtr( ) );
		int i = 0;
		for ( ; i < c_size; ++i ) 
			if ( reader[ i ] != static_cast< char >( i ) ) 
				break;
		EXPECT_FALSE( i < c_size );
	}

	CSharedMem::Free( sharedMemReader );
	CSharedMem::Free( sharedMemWriter );
}

} // namespace testSharedMemory_ 
