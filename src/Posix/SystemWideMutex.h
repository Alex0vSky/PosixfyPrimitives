// src/Posix/SystemWideMutex.h - ipc mutex facility
#pragma once // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
namespace Ipc {
class CSystemWideMutex {
	sem_t *h_mutex;
	const std::string m_name;

public:
	CSystemWideMutex(const char *name,bool *p_already_exists=NULL,bool open_existing=false) :
		h_mutex( SEM_FAILED )
		, m_name( std::string( "\\" ) + name )
	{
		bool is_exists = false;
		int mode = 0644;
		int value = 1;
		h_mutex = sem_open( m_name.c_str( ), O_CREAT | O_EXCL, mode, value );
		if ( SEM_FAILED == h_mutex ) {
			is_exists = ( EEXIST == errno );
			h_mutex = sem_open( m_name.c_str( ), O_CREAT, mode, value );
		}
		if ( open_existing && !is_exists )
			h_mutex = SEM_FAILED;

		if ( p_already_exists )
			*p_already_exists = is_exists;
	}
	//sem_wait( h_mutex ); //sem_post( h_mutex );

	CSystemWideMutex(const CSystemWideMutex& other) {
		//h_mutex = CTools::CopyHandle(other.h_mutex);
	}

	const CSystemWideMutex& operator = (const CSystemWideMutex& other) {
		if ( this != &other )
		{
			//CTools::CloseAndInvalidateHandle(h_mutex);
			//h_mutex = CTools::CopyHandle(other.h_mutex);
		}

		return *this;
	}

	~CSystemWideMutex() {
		// TODO(alex): via iface `CTools::CloseAndInvalidateHandle(h_mutex);`
		sem_unlink( m_name.c_str( ) );
	}

	bool IsError() const {
		return h_mutex == SEM_FAILED;
	}
							
	// returns true if we've got ownership, so Unlock() must be called when ownership is no longer needed
	// it is safe to call Unlock() without corresp. Lock() returned true
	bool Lock(unsigned timeout_ms)  {
		//if ( !h_mutex )
		//{
			return false;
		//}
		//else
		//{
		//	DWORD wc = WaitForSingleObject(h_mutex,timeout_ms);
		//	return wc == WAIT_OBJECT_0 || wc == WAIT_ABANDONED;
		//}
	}

	bool LockInfinite() {
		return Lock(INFINITE);
	}
							
	// it is safe to call Unlock() without corresp. Lock() returned true
	void Unlock() {
		if ( h_mutex )
		{
			//ReleaseMutex(h_mutex);
		}
	}
};
} // namespace Ipc
