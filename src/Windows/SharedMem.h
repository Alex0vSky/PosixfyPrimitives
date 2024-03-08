// src/Windows/SharedMem.h - shared memory facility 
#pragma once // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
namespace Ipc {
class CSharedMem {
    HANDLE h_map;
    void *m_buff;

    CSharedMem(const char *_name, bool *_p_already_exists, bool open_existing, unsigned size) : 
		h_map( NULL )
		, m_buff( nullptr )
    {
		std::string name = std::string("Global\\") + _name;

		// Make sure we have access to global
		LUID luid = { };
		PRIVILEGE_SET privs = { 1, PRIVILEGE_SET_ALL_NECESSARY, { luid, SE_PRIVILEGE_ENABLED } };
		HANDLE hToken = INVALID_HANDLE_VALUE;
		BOOL bResult = FALSE;
		if ( false
			|| !::LookupPrivilegeValue( NULL, SE_CREATE_GLOBAL_NAME, &luid ) // SE_CHANGE_NOTIFY_NAME
			|| ( privs.Privilege[0].Luid = luid, false ) // just assign
			|| !::OpenProcessToken( ::GetCurrentProcess( ), TOKEN_QUERY, &hToken )
			|| !::PrivilegeCheck( hToken, &privs, &bResult )
			|| !bResult
		)
			name = std::string("Local\\") + _name;
		if ( INVALID_HANDLE_VALUE != hToken )
			::CloseHandle( hToken );

		bool is_exists;
		if ( !open_existing ) {
			h_map = ::CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE,0,size,name.c_str());
			is_exists = (::GetLastError() == ERROR_ALREADY_EXISTS || ::GetLastError() == ERROR_ACCESS_DENIED);
		} else {
			h_map = ::OpenFileMappingA(FILE_MAP_READ|FILE_MAP_WRITE,FALSE,name.c_str());
			is_exists = (h_map != NULL || ::GetLastError() == ERROR_ACCESS_DENIED);
		}

		if ( h_map ) 
			m_buff = ::MapViewOfFile( h_map, FILE_MAP_READ|FILE_MAP_WRITE, 0, 0, 0 );

		if ( _p_already_exists ) 
            *_p_already_exists = is_exists;
    }

    ~CSharedMem() {
		if ( m_buff ) 
            ::UnmapViewOfFile( m_buff ), m_buff = nullptr;
		if ( h_map ) 
            ::CloseHandle( h_map ), h_map = NULL;
    }

public:
    static CSharedMem* Create(const char *name, unsigned size, bool *_p_already_exists = nullptr) {
	    return new CSharedMem( name, _p_already_exists, false, size );
    }
    static CSharedMem* Open(const char *name) {
		return new CSharedMem( name, nullptr, true, 0 );
    }
    static void Free(CSharedMem* &obj) {
		if ( obj ) 
            delete obj, obj = nullptr;
    }
    bool IsError() const {
		return GetMemPtr( ) == nullptr;
    }
    void* GetMemPtr() const {
		return m_buff;
    }
};
} // namespace Ipc
