// tests/testProcess.cpp - test posixfy process // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
#include "CTools.h"
#ifdef _WIN32
#	include "Windows/Process.h"
#else
#	include "Posix/Process.h"
#endif
using CProcess = Ipc::CProcess;

//*
namespace testProcess_ { 

TEST(Process_create, simple) {
#ifdef WIN32
	CProcess *proc = CProcess::Create( "ping 8.8.8.8" );
#else
	CProcess *proc = CProcess::Create( "ping -с1 8.8.8.8" );
#endif // WIN32
	EXPECT_FALSE( proc ->IsError( ) );
	while ( proc ->IsProcessActive( ) )
		std::this_thread::yield( );
}

/*
TEST(Process_create, simple_cwd) {
	//std::vector< char > buffer( 1024 );
	//getcwd( buffer.data( ), buffer.size( ) );
	//std::string cwd( buffer.begin( ), buffer.end( ) );

    std::string tmpnam = std::tmpnam( nullptr );
	size_t pos;
	pos = tmpnam.find_last_of( '/' );
	if ( std::string::npos == pos )
		pos = tmpnam.find_last_of( '\\' );
	if ( std::string::npos == pos )
		GTEST_SKIP( );
    std::string directory = tmpnam.substr( 0, pos );
	printf( "directory: '%s'\n", directory.c_str( ) );

	// get current directory shell command
#ifdef WIN32
	std::string cmdline = "cmd /c echo %cd%";
#else
	std::string cmdline = "pwd";
#endif // WIN32

	testing::internal::CaptureStdout( );
	CProcess *proc = CProcess::Create( cmdline.c_str( ), directory.c_str( ) );
	while ( proc ->IsProcessActive( ) )
		std::this_thread::yield( );
	std::string output = testing::internal::GetCapturedStdout( );
	printf( "output: '%s'\n", output.c_str( ) );

	EXPECT_FALSE( proc ->IsError( ) );
	EXPECT_NE( std::string::npos, output.find( directory ) );
}
//*/

/*
TEST(Process_create, xxx) {
	//getcwd( );
	CProcess *proc = CProcess::Create( "cmd /c dir" );
	int perr;
	//EXPECT_FALSE( proc ->IsError( &perr ) );
	proc ->IsError( &perr );
	printf( "perr: %d\n", perr );
	EXPECT_TRUE( proc ->IsProcessActive( ) );
	while ( proc ->IsProcessActive( ) )
		std::this_thread::yield( );
	int ec = 0;
	EXPECT_TRUE( proc ->GetExitCode( ec ) );
	printf( "ec: %d\n", ec );
	EXPECT_NE( nullptr, proc );
}
//*/

} // namespace testProcess_ 
//*/
