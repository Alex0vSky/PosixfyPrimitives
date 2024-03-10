// src/Posix/Process.h - process facility
#pragma once // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
namespace Ipc { class CProcess {
public:
	typedef unsigned process_id_t;

private:
	// child_pid
	pid_t h_process;
	process_id_t m_id_process;
	mutable int m_err;
	mutable bool m_reaped_exit_code;

	static const unsigned c_maximumPathLength = PATH_MAX;
	static const pid_t c_invalid = 0;
	static constexpr auto now = std::chrono::high_resolution_clock::now;

public:
	static CProcess* Create(const char *cmdline, const char *cwd=nullptr)
	{
		return new CProcess(cmdline,cwd);
	}
	static CProcess* Open(process_id_t pid,bool terminate_access_needed) {
		return new CProcess( pid );
	}

	// returns true if no timeout occurs
	static bool TerminateWaitDestroy(CProcess*& obj,unsigned wait_timeout_milli) {
		bool rc = obj ? obj ->_TerminateWaitDestroy( wait_timeout_milli ) : true;
		obj = nullptr;
		return rc;
	}
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
							
	// @insp SO/how-to-check-if-a-process-is-running-in-c
	// @insp SO/get-exit-code-from-non-child-process-in-linux
	bool IsProcessActive(unsigned wait_milli=0) const {
		auto next_clock = now( ) + std::chrono::milliseconds{ wait_milli };
		do {
			int status;
			// Wait for child process, this should clean up defunct processes
			if ( -1 == waitpid( h_process, &status, WNOHANG ) ) {
				// TODO(alex): just to known
				perror( "waitpid IsProcessActive" );
			}
			// save exit code, can wait for a process only once
			if ( WIFEXITED( status ) ) {
				m_reaped_exit_code = true;
				m_err = WEXITSTATUS( status );
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

	// @insp SO/get-exit-code-from-non-child-process-in-linux
	bool GetExitCode(int& _ec) const {
		if ( m_reaped_exit_code )
			return _ec = m_err, true;

		if ( c_invalid == h_process )
			return false;
		int status;
		if ( -1 == waitpid( h_process, &status, WNOHANG | WUNTRACED | WCONTINUED ) ) {
			// TODO(alex): just to known
			perror( "waitpid GetExitCode" );
		}
		printf( "WIFEXITED( status ): %s\n", ( WIFEXITED( status ) ?"true" :"false" ) );
		if ( !WIFEXITED( status ) )
			return false;
		_ec = m_err = WEXITSTATUS( status );
		return true;
	}

	process_id_t GetProcessId() const {
		return m_id_process;
	}

	static process_id_t GetThisProcessId() {
		return (process_id_t)getpid( );
	}

	static process_id_t GetInvalidProcessId() {
		return (process_id_t)-1;
	}

private:
	CProcess(process_id_t pid) {
		m_id_process = pid;
		h_process = pid;
		m_err = -1;
	}

	CProcess(const char *_cmdline,const char *cwd) : 
		h_process( c_invalid )
		, m_id_process( GetInvalidProcessId( ) )
		, m_err( -1 )
		, m_reaped_exit_code( false )
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
		//const char *const argv_[] = { "sh", "-c", cmdline.data( ), nullptr };
		//const char *const argv_[] = { cmdline.data( ), nullptr };
		const char *const argv_[] = { "ping", "-w 2 8.8.8.8", cmdline.data( ), nullptr };
		auto argv = const_cast< char *const*>( argv_ );
		if ( posix_spawnp( &h_process, argv[ 0 ], &action, nullptr, argv, environ ) ) {
			h_process = c_invalid;
			m_err = errno;
			perror( "posix_spawnp" );
			return;
		}
		m_id_process = h_process;
	}

	~CProcess() {
		if ( c_invalid != h_process )
			h_process = c_invalid;
	}

	// returns true if no timeout occurs
	bool _TerminateWaitDestroy(unsigned wait_timeout_milli) { 
		struct Deleter { 
			CProcess *x; Deleter(CProcess *y) : x( y ) { } ~Deleter() { delete x ; }
		} unused_( this );

		//printf( "IsProcessActive BEG\n" );
		//if ( IsProcessActive( ) ) {
		//	int status = kill( h_process, SIGKILL );
		//	printf( "do kill, status: %d\n", status );
		//	printf( "do kill, errno: %d\n", errno );
		//}
		//printf( "IsProcessActive END\n" );

		if ( !kill( h_process, 0 ) ) {
			int status = kill( h_process, SIGKILL );
			printf( "do2 kill, status: %d\n", status );
			printf( "do2 kill, errno: %d\n", errno );
		}

		// Awaiting free
		auto next_clock = now( ) + std::chrono::milliseconds{ wait_timeout_milli };
		do {
			// @Warning! Race condition by pid number unique
			//if ( ( kill( h_process, 0 ) == -1 ) && ( errno == ESRCH ) )
			// EINVAL

			static bool s_once1 = false;
			if ( !s_once1 )
				s_once1 = true, printf( "BEFORE kill errno: %d\n", errno ), perror( "BEFORE kill" );
			int status = kill( h_process, 0 );

			static bool s_once3 = false;
			if ( !s_once3 )
				s_once3 = true, printf( "kill status: %d\n", status );

			if ( status == -1 ) {
				if ( errno == ESRCH || errno == ENOENT )
					return true;
			}
			static bool s_once2 = false;
			if ( !s_once2 )
				s_once2 = true, printf( "AFTER kill errno: %d\n", errno ), perror( "AFTER kill" );
			//std::this_thread::yield( );
			std::this_thread::sleep_for( std::chrono::milliseconds{ 100 } );
		} while ( now( ) < next_clock );

		return false;
	}

	//void _DestroyNoTerminate() {
	//	delete this;
	//}
};

} // namespace Ipc
