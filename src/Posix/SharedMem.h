﻿// src/Posix/SharedMem.h - shared memory facility
#pragma once // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
namespace Ipc {
class CSharedMem {
    //HANDLE h_map;
    void *m_buff;
	const std::string m_name;
	const unsigned m_size;

    CSharedMem(const char *_name, bool *_p_already_exists, bool open_existing, unsigned size) : 
		m_buff( nullptr )
		//h_map( NULL )
		, m_name( std::string("/") + _name )
		, m_size( size )
    {
		bool is_exists;
		int fd = -1;
		if ( !open_existing ) {
			//h_map = ::CreateFileMappingA(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,size,name.c_str());
			//is_exists = (::GetLastError() == ERROR_ALREADY_EXISTS || ::GetLastError() == ERROR_ACCESS_DENIED);
			do { 
				fd = shm_open( m_name.c_str( ), O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH );
				if ( -1 == fd ) {
					is_exists = ( EEXIST == errno );

					// tmp
					if ( !is_exists ) 
						errExit( "shm_open" );

					break;
				}
				if ( -1 == ftruncate( fd, size ) ) {

					// tmp
					errExit("ftruncate");

					break;
				}
			} while ( false );
		} else {
			//h_map = ::OpenFileMappingA(FILE_MAP_READ|FILE_MAP_WRITE,FALSE,name.c_str());
			//is_exists = (h_map != NULL || ::GetLastError() == ERROR_ACCESS_DENIED);
		}

		if ( -1 != fd ) {
			m_buff = mmap( NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 ); 
			
			// tmp
			if ( MAP_FAILED == m_buff )
				 errExit("mmap");
		}

		if ( _p_already_exists ) 
            *_p_already_exists = is_exists;
    }

    ~CSharedMem() {
		//if ( m_buff ) 
		//	::UnmapViewOfFile( m_buff ), m_buff = nullptr;
		//if ( h_map ) 
		//	::CloseHandle( h_map ), h_map = NULL;
		if ( m_buff ) 
			munmap( m_buff, m_size ), m_buff = nullptr, shm_unlink( m_name.c_str( ) );
    }

public:
    static CSharedMem* Create(const char *name, unsigned size, bool *_p_already_exists = nullptr) {
	    return new CSharedMem( name, _p_already_exists, false, size );
    }
  //  static CSharedMem* Open(const char *name) {
		//return new CSharedMem( name, nullptr, true, 0 );
  //  }
    static void Free(CSharedMem* &obj) {
		if ( obj ) 
            delete obj, obj = nullptr;
    }
  //  bool IsError() const {
		//return GetMemPtr( ) == nullptr;
  //  }
  //  void* GetMemPtr() const {
		//return m_buff;
  //  }
};
} // namespace Ipc
