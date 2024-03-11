// src/Posix/Process.h - process facility
#pragma once // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
namespace Ipc { 
class CProcess {
public:
	typedef unsigned process_id_t;

private:
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
	static CProcess* Open(process_id_t pid,bool) {
		return new CProcess( pid );
	}

	// returns true if no timeout occurs
	static bool TerminateWaitDestroy(CProcess*& obj,unsigned wait_timeout_milli) {
		bool rc = obj ? obj ->_TerminateWaitDestroy( wait_timeout_milli ) : true;
		obj = nullptr;
		return rc;
	}
	static void DestroyNoTerminate(CProcess*& obj) {
		if ( !obj )
			return;
		obj ->_DestroyNoTerminate( );
		obj = nullptr;
	}

	bool IsError(int *_perr=nullptr) const {
		if ( _perr )
			*_perr = m_err;
		return h_process == c_invalid;
	}
							
	// @insp SO/how-to-check-if-a-process-is-running-in-c
	// @insp SO/get-exit-code-from-non-child-process-in-linux
	bool IsProcessActive(unsigned wait_milli=0) const {
		auto next_clock = now( ) + std::chrono::milliseconds{ wait_milli };
		do {
			int status = 0;
			waitpid( h_process, &status, WNOHANG );
			if ( WIFEXITED( status ) ) {
				m_reaped_exit_code = true;
				m_err = WEXITSTATUS( status );
			}
			if ( kill( h_process, 0 ) == -1 ) {
				if ( errno == ESRCH )
					return false;
			}
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
		int status = 0;
		if ( -1 == waitpid( h_process, &status, WNOHANG ) ) 
			return false;
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
		const char *const argv_[] = { "sh", "-c", cmdline.data( ), nullptr };
		auto argv = const_cast< char *const*>( argv_ );
		if ( posix_spawnp( &h_process, argv[ 0 ], &action, nullptr, argv, environ ) ) {
			h_process = c_invalid;
			m_err = errno;
			return;
		}
		m_id_process = h_process;
	}

	~CProcess() {
		if ( c_invalid != h_process )
			waitpid( h_process, nullptr, 0 ), h_process = c_invalid;
	}

	// returns true if no timeout occurs
	bool _TerminateWaitDestroy(unsigned wait_timeout_milli) { 
		struct Deleter { 
			CProcess *x; Deleter(CProcess *y) : x( y ) { } ~Deleter() { delete x ; }
		} unused_( this );

		if ( !kill( h_process, 0 ) ) {
			kill( h_process, SIGKILL );
			// @insp SO/cant-kill-pid-started-with-spawn
			waitpid( h_process, nullptr, 0 );
		}

		// Awaiting free
		auto next_clock = now( ) + std::chrono::milliseconds{ wait_timeout_milli };
		do {
			// @Warning! Race condition by pid number
			if ( ( kill( h_process, 0 ) == -1 ) && ( errno == ESRCH ) )
				return true;
			std::this_thread::yield( );
		} while ( now( ) < next_clock );

		return false;
	}

	void _DestroyNoTerminate() {
		delete this;
	}
};

} // namespace Ipc
