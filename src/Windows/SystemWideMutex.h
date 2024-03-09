// src/Windows/SystemWideMutex.h - ipc mutex facility 
#pragma once // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
namespace Ipc {
class CSystemWideMutex {
	HANDLE h_mutex;

public:
	CSystemWideMutex(const char *name,bool *p_already_exists=NULL,bool open_existing=false) {
		const std::string prefix = "Global\\";

		bool is_exists;
								
		if ( open_existing )
		{
			h_mutex = OpenMutexA(SYNCHRONIZE,FALSE,name?(prefix+name).c_str():NULL);
			is_exists = (h_mutex != NULL || GetLastError() == ERROR_ACCESS_DENIED);
		}
		else
		{
			h_mutex = CreateMutexA(NULL,FALSE,name?(prefix+name).c_str():NULL);
			is_exists = (GetLastError() == ERROR_ALREADY_EXISTS || GetLastError() == ERROR_ACCESS_DENIED);
		}

		if ( p_already_exists )
		{
			*p_already_exists = is_exists;
		}
	}

	CSystemWideMutex(const CSystemWideMutex& other) {
		h_mutex = CTools::CopyHandle(other.h_mutex);
	}

	const CSystemWideMutex& operator = (const CSystemWideMutex& other) {
		if ( this != &other )
		{
			CTools::CloseAndInvalidateHandle(h_mutex);
			h_mutex = CTools::CopyHandle(other.h_mutex);
		}

		return *this;
	}

	~CSystemWideMutex() {
		CTools::CloseAndInvalidateHandle(h_mutex);
	}

	bool IsError() const {
		return h_mutex == NULL;
	}
							
	// returns true if we've got ownership, so Unlock() must be called when ownership is no longer needed
	// it is safe to call Unlock() without corresp. Lock() returned true
	bool Lock(unsigned timeout_ms)  {
		if ( !h_mutex )
		{
			return false;
		}
		else
		{
			DWORD wc = WaitForSingleObject(h_mutex,timeout_ms);
			return wc == WAIT_OBJECT_0 || wc == WAIT_ABANDONED;
		}
	}

	bool LockInfinite() {
		return Lock(INFINITE);
	}
							
	// it is safe to call Unlock() without corresp. Lock() returned true
	void Unlock() {
		if ( h_mutex )
		{
			ReleaseMutex(h_mutex);
		}
	}
};
} // namespace Ipc
