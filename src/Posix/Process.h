// src/Posix/Process.h - process facility
#pragma once // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
namespace Ipc { class CProcess {
public:
	typedef unsigned process_id_t;

private:
	// child_pid
	pid_t h_process;
	process_id_t m_id_process;
	int m_err;

	static const unsigned c_maximumPathLength = PATH_MAX;
	static const pid_t c_invalid = 0;
	static constexpr auto now = std::chrono::high_resolution_clock::now;

public:
	static CProcess* Create(const char *cmdline, const char *cwd=nullptr)
	{
		return new CProcess(cmdline,cwd);
	}
	//static CProcess* Open(process_id_t pid,bool terminate_access_needed)
	//{
	//	return new CProcess(pid,SYNCHRONIZE|(terminate_access_needed?PROCESS_TERMINATE:0));
	//}

	//// returns true if no timeout occurs
	//static bool TerminateWaitDestroy(CProcess*& obj,unsigned wait_timeout_ms) {
	//	bool rc = obj ? obj->_TerminateWaitDestroy(wait_timeout_ms) : true;
	//	obj = NULL;
	//	return rc;
	//}
	//static void DestroyNoTerminate(CProcess*& obj) {
	//	if ( obj )
	//	{
	//		obj->_DestroyNoTerminate();
	//		obj = NULL;
	//	}
	//}

	bool IsError(int *_perr=nullptr) const {
		if ( _perr )
		{
			*_perr = m_err;
		}
								
		return h_process == c_invalid;
	}
							
	// @insp https://stackoverflow.com/questions/45037193/how-to-check-if-a-process-is-running-in-c
	bool IsProcessActive(unsigned wait_milli=0) const {
		auto next_clock = now( ) + std::chrono::milliseconds{ wait_milli };
		do {
			// Wait for child process, this should clean up defunct processes
			if ( -1 == waitpid( h_process, nullptr, WNOHANG ) ) {
				// TODO(alex): just to known
				perror( "waitpid IsProcessActive" );
			}
			// kill failed let's see why..
			if ( kill( h_process, 0 ) == -1 ) {
				// First of all kill may fail with EPERM if we run as a different user and we have no access, 
				// so let's make sure the errno is ESRCH (Process not found!)
				if ( errno == ESRCH )
					return false;
			}
			// If kill didn't fail the process is still running
			std::this_thread::yield( );
		} while ( now( ) < next_clock );
		return true;
	}

	bool GetExitCode(int& _ec) const {
		if ( c_invalid == h_process )
			return false;
		int status;
		if ( -1 == waitpid( h_process, &status, WNOHANG ) ) {
			// TODO(alex): just to known
			perror( "waitpid GetExitCode" );
//			return false;
		}
		if ( !WIFEXITED( status ) )
			return false;
		_ec = WEXITSTATUS( status );
		return true;
	}

	//process_id_t GetProcessId() const {
	//	return m_id_process;
	//}

	//static process_id_t GetThisProcessId() {
	//	return (process_id_t)::GetCurrentProcessId();
	//}

	static process_id_t GetInvalidProcessId() {
		return (process_id_t)-1;
	}

private:
	//CProcess(process_id_t pid,DWORD access) {
	//	m_id_process = pid;
	//	h_process = OpenProcess(access,FALSE,pid);
	//	m_err = ::GetLastError();
	//}
							
	CProcess(const char *_cmdline,const char *cwd) : 
		h_process( c_invalid ), m_id_process(GetInvalidProcessId()), m_err(-1)
	{
		if ( !_cmdline )
			return;
		// is poorly supported SO/stdc-lib-ext1-availability-in-gcc-and-clang
#ifdef __STDC_LIB_EXT1__ 
		size_t len = strnlen_s( _cmdline, c_maximumPathLength );
#else
		size_t len = strnlen( _cmdline, c_maximumPathLength );
#endif // __STDC_LIB_EXT1__ 
		if ( !len )
			return;

		posix_spawn_file_actions_t action;
		if ( posix_spawn_file_actions_init( &action ) )
			return;
		struct Destroy { 
			posix_spawn_file_actions_t &x; Destroy(posix_spawn_file_actions_t &y) : x( y ) { } ~Destroy() { 
				posix_spawn_file_actions_destroy( &x ); }
		} unused_( action );
		if ( cwd ) 
			if ( posix_spawn_file_actions_addchdir_np( &action, cwd ) )
				return;

		std::vector< char > cmdline( _cmdline, _cmdline + len + 1 );
		const char *const argv_[] = { "sh", "-c", cmdline.data( ), nullptr };
		auto argv = const_cast< char *const*>( argv_ );
		if ( posix_spawnp( &h_process, argv[ 0 ], &action, nullptr, argv, environ ) ) {
			h_process = c_invalid;
			m_err = errno;
			perror( "posix_spawnp" );
		}
	}

	//~CProcess() {
	//	if ( h_process )
	//	{
	//		CloseHandle(h_process);
	//		h_process = NULL;
	//	}
	//}

	//bool _TerminateWaitDestroy(unsigned wait_timeout_ms)  // returns true if no timeout occurs
	//{
	//	if ( IsProcessActive() )
	//	{
	//		TerminateProcess(h_process,0);  // async
	//	}
	//							
	//	DWORD wc = (h_process ? WaitForSingleObject(h_process,wait_timeout_ms) : WAIT_FAILED);
	//	delete this;
	//	return wc == WAIT_OBJECT_0;
	//}

	//void _DestroyNoTerminate() {
	//	delete this;
	//}
};

} // namespace Ipc
