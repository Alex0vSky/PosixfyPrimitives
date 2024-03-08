﻿// src/Posix/SharedMem.h - shared memory facility
#pragma once // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
namespace Ipc {

// tmp 
#define errExit(msg) perror(msg)

class CSharedMem {
    void *m_buff;
	const std::string m_name;
	const unsigned m_size;

    CSharedMem(const char *_name, bool *_p_already_exists, bool open_existing, unsigned size) : 
		m_buff( nullptr )
		, m_name( std::string("/") + _name )
		, m_size( size )
    {
		const mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
		bool is_exists;
		int fd = -1;
		if ( !open_existing ) {
			fd = shm_open( m_name.c_str( ), O_CREAT | O_EXCL | O_RDWR, mode );
			if ( -1 != fd ) {
				if ( -1 == ftruncate( fd, size ) ) {

					// tmp
					errExit("ftruncate3");
				}
			}
			if ( -1 == fd ) {
				is_exists = ( EEXIST == errno );

				if ( is_exists )
					fd = shm_open( m_name.c_str( ), O_RDWR, 0 );
				//// tmp
				//if ( -1 == fd ) {
				//	errExit( "shm_open2" );
				//}
			}

			if ( -1 != fd ) {

				m_buff = mmap( NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 ); 
			
				// tmp
				if ( !m_buff ) {
					errExit("mmap11");
				}

				if ( MAP_FAILED == m_buff ) {
					m_buff = nullptr;
					// tmp
					errExit("mmap12");
				}
			}

		} else {
			fd = shm_open( m_name.c_str( ), O_RDWR, 0 );
			// tmp
			if ( -1 == fd ) {
				errExit( "shm_open21" );
			}
			if ( -1 != fd ) {

				m_buff = mmap( NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0 ); 
			
				// tmp
				if ( !m_buff ) {
					errExit("mmap22");
				}

				if ( MAP_FAILED == m_buff ) {
					m_buff = nullptr;
					// tmp
					errExit("mmap21");
				}
			}
		}

		if ( _p_already_exists ) 
            *_p_already_exists = is_exists;
    }

    ~CSharedMem() {
		if ( m_buff ) 
			munmap( m_buff, m_size ), m_buff = nullptr, shm_unlink( m_name.c_str( ) );
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
