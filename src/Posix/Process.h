// src/Posix/Process.h - process facility
#pragma once // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
namespace Ipc { class CProcess {
public:
	typedef unsigned process_id_t;

private:
	// child_pid
	pid_t *h_process;
	process_id_t m_id_process;
	int m_err;

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
								
		return h_process == nullptr;
	}
							
	//bool IsProcessActive(unsigned wait_ms=0) const {
	//	return h_process ? WaitForSingleObject(h_process,wait_ms) == WAIT_TIMEOUT : false;
	//}

	//bool GetExitCode(int& _ec) const {
	//	bool rc = false;
	//	if ( h_process )
	//	{
	//		DWORD ec = STILL_ACTIVE;
	//		if ( GetExitCodeProcess(h_process,&ec) && ec != STILL_ACTIVE )
	//		{
	//			rc = true;
	//			_ec = (int)ec;
	//		}
	//	}
	//	return rc;
	//}

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
		h_process( nullptr ), m_id_process(GetInvalidProcessId()), m_err(-1)
	{
		char *argv[] = { "sh", "-c", nullptr, nullptr };
		std::string cmdline2 = _cmdline;
		std::vector< char > cmdline( cmdline2.begin( ), cmdline2.end( ) );
		argv[ 2 ] = cmdline.data( );
		if ( posix_spawnp( h_process, argv[ 0 ], nullptr, nullptr, argv, environ ) ) {
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
