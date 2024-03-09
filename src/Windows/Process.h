// src/Windows/Process.h - process facility 
#pragma once // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
namespace Ipc {
// объект для создания/запуска дочернего процесса
class CProcess {
public:
	typedef unsigned process_id_t;

private:
	HANDLE h_process;
	process_id_t m_id_process;
	int m_err;

public:
	static CProcess* Create(const char *cmdline,const char *cwd=NULL)
	{
		return new CProcess(cmdline,cwd);
	}
	static CProcess* Open(process_id_t pid,bool terminate_access_needed)
	{
		return new CProcess(pid,SYNCHRONIZE|(terminate_access_needed?PROCESS_TERMINATE:0));
	}

	// returns true if no timeout occurs
	static bool TerminateWaitDestroy(CProcess*& obj,unsigned wait_timeout_ms) {
		bool rc = obj ? obj->_TerminateWaitDestroy(wait_timeout_ms) : true;
		obj = NULL;
		return rc;
	}
	static void DestroyNoTerminate(CProcess*& obj) {
		if ( obj )
		{
			obj->_DestroyNoTerminate();
			obj = NULL;
		}
	}

	bool IsError(int *_perr=NULL) const {
		if ( _perr )
		{
			*_perr = m_err;
		}
								
		return h_process == NULL;
	}
							
	bool IsProcessActive(unsigned wait_ms=0) const {
		return h_process ? WaitForSingleObject(h_process,wait_ms) == WAIT_TIMEOUT : false;
	}

	bool GetExitCode(int& _ec) const {
		bool rc = false;
		if ( h_process )
		{
			DWORD ec = STILL_ACTIVE;
			if ( GetExitCodeProcess(h_process,&ec) && ec != STILL_ACTIVE )
			{
				rc = true;
				_ec = (int)ec;
			}
		}
		return rc;
	}

	process_id_t GetProcessId() const {
		return m_id_process;
	}

	static process_id_t GetThisProcessId() {
		return (process_id_t)::GetCurrentProcessId();
	}

	static process_id_t GetInvalidProcessId() {
		return (process_id_t)-1;
	}

private:
	CProcess(process_id_t pid,DWORD access) {
		m_id_process = pid;
		h_process = OpenProcess(access,FALSE,pid);
		m_err = ::GetLastError();
	}
							
	CProcess(const char *_cmdline,const char *cwd) : 
		h_process(NULL), m_id_process(GetInvalidProcessId()), m_err(-1)
	{
		std::string cmdline = (_cmdline ? _cmdline : "");  // need to copy cmdline
		if ( !cmdline.empty() ) {
			PROCESS_INFORMATION pi = {0,};
			STARTUPINFOA si;
			ZeroMemory(&si,sizeof(si));
			si.cb = sizeof(si);

			UINT err_mode = SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX|SEM_NOOPENFILEERRORBOX);

			BOOL rc;
			cmdline += '\0';  // paranoia
			rc = CreateProcessA(NULL,&cmdline[0],NULL,NULL,FALSE,0,NULL,cwd,&si,&pi);
									 
			m_err = ::GetLastError();
			SetErrorMode(err_mode);

			if ( rc )
			{
				CloseHandle(pi.hThread);

				h_process = pi.hProcess;
				m_id_process = pi.dwProcessId;
				m_err = 0;
			}
		}
	}

	~CProcess() {
		if ( h_process )
		{
			CloseHandle(h_process);
			h_process = NULL;
		}
	}

	bool _TerminateWaitDestroy(unsigned wait_timeout_ms)  // returns true if no timeout occurs
	{
		if ( IsProcessActive() )
		{
			TerminateProcess(h_process,0);  // async
		}
								
		DWORD wc = (h_process ? WaitForSingleObject(h_process,wait_timeout_ms) : WAIT_FAILED);

		delete this;

		return wc == WAIT_OBJECT_0;
	}

	void _DestroyNoTerminate() {
		delete this;
	}
};

} // namespace Ipc
