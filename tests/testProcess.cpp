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
	const char *g_long_playing = "ping -w 2 8.8.8.8";
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

/*
TEST(Process_create, simple) {
	SilenceStdout anonimous_;
	CProcess *proc = CProcess::Create( g_long_playing );
	EXPECT_FALSE( proc ->IsError( ) );
}


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

TEST(Process_alive, infinite) {
	SilenceStdout anonimous_;
	CProcess *proc = CProcess::Create( g_long_playing );
	EXPECT_FALSE( proc ->IsProcessActive( INFINITE ) );
	EXPECT_FALSE( proc ->IsError( ) );
}

/*
TEST(Process_alive, immediately) {
	SilenceStdout anonimous_;
	CProcess *proc = CProcess::Create( g_long_playing );
	EXPECT_TRUE( proc ->IsProcessActive( ) );
	EXPECT_FALSE( proc ->IsError( ) );
}

TEST(Process_alive, timeout) {
	SilenceStdout anonimous_;
	CProcess *proc = CProcess::Create( g_long_playing );
	// more then two pings
	EXPECT_TRUE( proc ->IsProcessActive( 300 ) );
	EXPECT_FALSE( proc ->IsError( ) );
}

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
//*/

// Warning! static variable
class PrinterToStderr {
	class EventListener : public testing::EmptyTestEventListener {
		void OnTestPartResult(const testing::TestPartResult& test_part_result) override {
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

	std::unique_ptr< EventListener > m_eventListener;
	static testing::TestEventListener *s_original;

public:
	// set printer
	PrinterToStderr() :
		m_eventListener( std::make_unique< EventListener >( ) )
	{
		auto& listeners = testing::UnitTest::GetInstance( ) ->listeners( );
		if ( !s_original )
			s_original = listeners.default_result_printer( );
		listeners.Release( s_original );
		listeners.Append( m_eventListener.get( ) );
	}
	// restore default printer
	~PrinterToStderr() {
		auto& listeners = testing::UnitTest::GetInstance( ) ->listeners( );
		listeners.Release( m_eventListener.get( ) );
		listeners.Append( s_original );
	}
};
testing::TestEventListener *PrinterToStderr::s_original = nullptr;

/*
TEST(Process_exit_code, immediately_for_long_playing) {
	PrinterToStderr anonimous2_;
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
}
//*/

/*
TEST(Process_mix, pid) {
	EXPECT_TRUE( ( CProcess::GetThisProcessId( ) > 0 ) );
}

TEST(Process_open, basic) {
	PrinterToStderr anonimous2_;
	SilenceStdout anonimous_;
	CProcess *proc1 = CProcess::Create( g_long_playing );
	EXPECT_FALSE( proc1 ->IsError( ) );
	CProcess *proc2 = CProcess::Open( proc1 ->GetProcessId( ), false );
	EXPECT_FALSE( proc2 ->IsError( ) );
	EXPECT_TRUE( proc2 ->IsProcessActive( ) );
	EXPECT_EQ( proc1 ->GetProcessId( ), proc2 ->GetProcessId( ) );
	// memory leaks
}

TEST(Process_terminate, basic) {
	CProcess *proc = CProcess::Create( g_long_playing );
	EXPECT_FALSE( proc ->IsError( ) );
	EXPECT_TRUE( CProcess::TerminateWaitDestroy( proc, INFINITE ) );
	EXPECT_EQ( nullptr, proc );
	EXPECT_FALSE( proc ->IsProcessActive( ) );
}

//TEST(Process_terminate, then_terminate_pair) {
//	//PrinterToStderr anonimous2_;
//	//SilenceStdout anonimous_;
//	CProcess *proc1 = CProcess::Create( g_long_playing );
//	EXPECT_FALSE( proc1 ->IsError( ) );
//	CProcess *proc2 = CProcess::Open( proc1 ->GetProcessId( ), true );
//	EXPECT_FALSE( proc2 ->IsError( ) );
//	//EXPECT_TRUE( CProcess::TerminateWaitDestroy( proc1, 300 ) );
//	EXPECT_TRUE( CProcess::TerminateWaitDestroy( proc1, INFINITE ) );
//	EXPECT_EQ( nullptr, proc1 );
//	EXPECT_FALSE( proc2 ->IsProcessActive( ) );
//}

//TEST(Process_terminate, double_terminate) {
//	PrinterToStderr anonimous2_;
//	SilenceStdout anonimous_;
//	CProcess *proc = CProcess::Create( g_long_playing );
//	EXPECT_TRUE( CProcess::TerminateWaitDestroy( proc, INFINITE ) );
//	EXPECT_EQ( nullptr, proc );
//	EXPECT_TRUE( CProcess::TerminateWaitDestroy( proc, INFINITE ) );
//}

//*/

/*
TEST(Process_open, then_actions) {
}

TEST(Process_delete, unaccesible?) {
}
//*/

} // namespace testProcess_ 
//*/
