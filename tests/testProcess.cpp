// tests/testProcess.cpp - test posixfy process // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
#include "CTools.h"
#ifdef _WIN32
#	include "Windows/Process.h"
#else
#	include "Posix/Process.h"
#endif
using CProcess = Ipc::CProcess;
// Long-playing command, to ping Google via gtest
#ifdef WIN32
	const char *g_long_playing = "ping -n 2 8.8.8.8";
#else
	const char *g_long_playing = "ping -c 2 8.8.8.8";
#endif // WIN32

//*
namespace testProcess_ { 

struct SilenceStdout {
	SilenceStdout() {
		testing::internal::CaptureStdout( );
	}
	~SilenceStdout() {
		testing::internal::GetCapturedStdout( );
	}
};
TEST(Process_create, simple) {
	SilenceStdout anonimous_;
	CProcess *proc = CProcess::Create( g_long_playing );
	EXPECT_FALSE( proc ->IsError( ) );
}

//*
TEST(Process_create, set_and_verify_real_cwd) {
    std::string tmpnam = std::tmpnam( nullptr );
	size_t pos;
	pos = tmpnam.find_last_of( '/' );
	if ( std::string::npos == pos )
		pos = tmpnam.find_last_of( '\\' );
	if ( std::string::npos == pos )
		GTEST_SKIP( );
    std::string directory = tmpnam.substr( 0, pos );

	// get current directory shell command
#ifdef WIN32
	std::string cmdline = "cmd /c echo %cd%";
#else
	std::string cmdline = "echo $(pwd)";
#endif // WIN32

	testing::internal::CaptureStdout( );
	CProcess *proc = CProcess::Create( cmdline.c_str( ), directory.c_str( ) );
	while ( proc ->IsProcessActive( ) )
		std::this_thread::yield( );
	std::string output = testing::internal::GetCapturedStdout( );

	EXPECT_FALSE( proc ->IsError( ) );
	EXPECT_NE( std::string::npos, output.find( directory ) );
}
//*/

/*
TEST(Process_alive, immediately) {
	SilenceStdout anonimous_;
	CProcess *proc = CProcess::Create( g_long_playing );
	EXPECT_TRUE( proc ->IsProcessActive( ) );
	EXPECT_FALSE( proc ->IsError( ) );
}

TEST(Process_alive, infinite) {
	SilenceStdout anonimous_;
	CProcess *proc = CProcess::Create( g_long_playing );
	EXPECT_FALSE( proc ->IsProcessActive( INFINITE ) );
	EXPECT_FALSE( proc ->IsError( ) );
}

TEST(Process_alive, timeout) {
	SilenceStdout anonimous_;
	CProcess *proc = CProcess::Create( g_long_playing );
	// more then two pings
	EXPECT_TRUE( proc ->IsProcessActive( 300 ) );
	EXPECT_FALSE( proc ->IsError( ) );
}
//*/

TEST(Process_exit_code, for_finished) {
	// set process return code shell command
#ifdef WIN32
	std::string cmdline = "cmd /c exit 42";
#else
	std::string cmdline = "exit 42";
#endif // WIN32

	CProcess *proc = CProcess::Create( cmdline.c_str( ) );
	proc ->IsProcessActive( INFINITE );
	EXPECT_FALSE( proc ->IsError( ) );
	int exit_code;
	EXPECT_TRUE( proc ->GetExitCode( exit_code ) );
	EXPECT_EQ( 42, exit_code );
}

class PrinterToStderr : public testing::EmptyTestEventListener {
	void OnTestPartResult(const testing::TestPartResult& test_part_result) override {
		//// gtest-port.cc(1153):: Only one stdout capturer can exist at a time.
		//testing::internal::CaptureStdout( );
		//m_p ->OnTestPartResult( test_part_result );
		//std::cerr << testing::internal::GetCapturedStdout( );

		std::vector< char > buffer( 4096 );
		snprintf( buffer.data( ), buffer.size( )
				, "%s in %s:%d\n%s\n",
				test_part_result.failed() ? "*** Failure" : "Success",
				test_part_result.file_name(),
				test_part_result.line_number(),
				test_part_result.summary());
		std::cerr << std::string( buffer.begin( ), buffer.end( ) );
	}
};

TEST(Process_exit_code, immediately_for_long_playing) {

	// set printer
	auto& listeners = testing::UnitTest::GetInstance( ) ->listeners( );
	auto stderr_result_printer = std::make_unique< PrinterToStderr >( );
	auto default_result_printer = listeners.Release( listeners.default_result_printer( ) );
	listeners.Append( stderr_result_printer.get( ) );

	SilenceStdout anonimous_;

	// set process return code shell command
#ifdef WIN32
	std::string cmdline = "cmd /c ping -n 2 8.8.8.8 && exit 42";
#else
	std::string cmdline = "ping -n 2 8.8.8.8 && exit 42";
#endif // WIN32

	CProcess *proc = CProcess::Create( cmdline.c_str( ) );
	EXPECT_FALSE( proc ->IsError( ) );
	int exit_code;
	EXPECT_FALSE( proc ->GetExitCode( exit_code ) );

	// restore default printer
	listeners.Release( stderr_result_printer.get( ) );
	listeners.Append( default_result_printer );
}

/*


TEST(Process_open, basic) {
}

TEST(Process_open, then_terminate) {
}

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
