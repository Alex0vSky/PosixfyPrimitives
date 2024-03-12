// tests/testProcess.cpp - test posixfy process // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
#include "CTools.h"
#ifdef _WIN32
#	include "Windows/Process.h"
#else
#	include "Posix/Process.h"
#endif
using CProcess = Ipc::CProcess;
// Long-playing command, to ping Google via gtest
#ifdef _WIN32
	const char *g_long_playing = "ping -n 2 8.8.8.8";
#else
	const char *g_long_playing = "ping -c 2 8.8.8.8";
#endif // _WIN32

//*
namespace testProcess_ { 

namespace detail {
struct SilenceStdout {
	SilenceStdout() {
		testing::internal::CaptureStdout( );
	}
	~SilenceStdout() {
		testing::internal::GetCapturedStdout( );
	}
};
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
} // namespace detail 

#ifdef _WIN32
typedef std::tuple< detail::SilenceStdout, detail::PrinterToStderr > 
	silenceStdoutAndPinterToStderr_t;
#else
// dummy
typedef void *
	silenceStdoutAndPinterToStderr_t;
#endif // _WIN32

TEST(Process_create, simple) {
	silenceStdoutAndPinterToStderr_t anonimous_;
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
#ifdef _WIN32
	std::string cmdline = "cmd /c echo %cd%";
#else
	std::string cmdline = "echo $(pwd)";
#endif // _WIN32

	testing::internal::CaptureStdout( );
	CProcess *proc = CProcess::Create( cmdline.c_str( ), directory.c_str( ) );
	while ( proc ->IsProcessActive( ) )
		std::this_thread::yield( );
	std::string output = testing::internal::GetCapturedStdout( );

	EXPECT_FALSE( proc ->IsError( ) );
	EXPECT_NE( std::string::npos, output.find( directory ) );
}

TEST(Process_alive, awaiting_until_end) {
	silenceStdoutAndPinterToStderr_t anonimous_;
	CProcess *proc = CProcess::Create( g_long_playing );
	EXPECT_FALSE( proc ->IsProcessActive( INFINITE ) );
	EXPECT_FALSE( proc ->IsError( ) );
	// memory leaks
}

TEST(Process_alive, immediately) {
	silenceStdoutAndPinterToStderr_t anonimous_;
	CProcess *proc = CProcess::Create( g_long_playing );
	EXPECT_TRUE( proc ->IsProcessActive( ) );
	EXPECT_FALSE( proc ->IsError( ) );
	// memory leaks
}

TEST(Process_alive, timeout) {
	silenceStdoutAndPinterToStderr_t anonimous_;
	CProcess *proc = CProcess::Create( g_long_playing );
	// less then will execute
	EXPECT_TRUE( proc ->IsProcessActive( 300 ) );
	EXPECT_FALSE( proc ->IsError( ) );
	// memory leaks
}

TEST(Process_exit_code, for_finished) {
	// set process return code shell command
#ifdef _WIN32
	std::string cmdline = "cmd /c exit 42";
#else
	std::string cmdline = "exit 42";
#endif // _WIN32

	CProcess *proc = CProcess::Create( cmdline.c_str( ) );
	// less then will execute
	proc ->IsProcessActive( 300 );
	EXPECT_FALSE( proc ->IsError( ) );
	int exit_code;
	EXPECT_TRUE( proc ->GetExitCode( exit_code ) );
	EXPECT_EQ( 42, exit_code );
	// memory leaks
}

TEST(Process_exit_code, immediately_for_long_playing) {
	silenceStdoutAndPinterToStderr_t anonimous_;

	// set process return code shell command
#ifdef _WIN32
	std::string cmdline = "cmd /c ping -n 2 8.8.8.8 && exit 42";
#else
	std::string cmdline = "ping -c 2 8.8.8.8 && exit 42";
#endif // _WIN32

	CProcess *proc = CProcess::Create( cmdline.c_str( ) );
	EXPECT_FALSE( proc ->IsError( ) );
	int exit_code;
	EXPECT_FALSE( proc ->GetExitCode( exit_code ) );
	// memory leaks
}

TEST(Process_mix, current_pid) {
	EXPECT_TRUE( ( CProcess::GetThisProcessId( ) > 0 ) );
}

TEST(Process_mix, invalid_pid) {
	EXPECT_NE( CProcess::GetThisProcessId( ), CProcess::GetInvalidProcessId( ) );
}

TEST(Process_open, basic) {
	silenceStdoutAndPinterToStderr_t anonimous_;
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
	EXPECT_TRUE( CProcess::TerminateWaitDestroy( proc, 60 *1000 ) );
	EXPECT_EQ( nullptr, proc );
}

TEST(Process_terminate, then_terminate_pair) {
	silenceStdoutAndPinterToStderr_t anonimous_;
	CProcess *proc1 = CProcess::Create( g_long_playing );
	EXPECT_FALSE( proc1 ->IsError( ) );
	CProcess *proc2 = CProcess::Open( proc1 ->GetProcessId( ), true );
	EXPECT_FALSE( proc2 ->IsError( ) );
	EXPECT_TRUE( CProcess::TerminateWaitDestroy( proc1, 300 ) );
	EXPECT_EQ( nullptr, proc1 );
	EXPECT_FALSE( proc2 ->IsProcessActive( ) );
	// memory leaks
}

TEST(Process_terminate, double_terminate) {
	silenceStdoutAndPinterToStderr_t anonimous_;
	CProcess *proc = CProcess::Create( g_long_playing );
	EXPECT_TRUE( CProcess::TerminateWaitDestroy( proc, 300 ) );
	EXPECT_EQ( nullptr, proc );
	EXPECT_TRUE( CProcess::TerminateWaitDestroy( proc, 300 ) );
}

TEST(Process_DestroyNoTerminate, basic) {
	silenceStdoutAndPinterToStderr_t anonimous_;
	CProcess *proc = CProcess::Create( g_long_playing );
	EXPECT_FALSE( proc ->IsError( ) );
	CProcess::DestroyNoTerminate( proc );
	EXPECT_EQ( nullptr, proc );
}

TEST(Process_traits, dtor_or_delete_unaccesible) {
	EXPECT_FALSE( std::is_destructible< CProcess >::value );
}

TEST(Process_traits, ctor_or_new_unaccesible) {
	EXPECT_FALSE( std::is_constructible< CProcess >::value );
}

} // namespace testProcess_ 
//*/
