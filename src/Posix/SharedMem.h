// src/Posix/SharedMem.h - ipc shared memory facility
#pragma once // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
namespace Ipc {
class CSharedMem {
    void *m_buff;
	const std::string m_name;
	unsigned m_size;

    CSharedMem(const char *_name, bool *_p_already_exists, bool open_existing, unsigned size) : 
		m_buff( MAP_FAILED )
		, m_name( std::string("/") + _name )
		, m_size( size )
    {
		const mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
		bool is_exists = false;
		int fd = -1;
		if ( !open_existing ) {
			fd = shm_open( m_name.c_str( ), O_CREAT | O_EXCL | O_RDWR, mode );
			if ( -1 == fd ) {
				if ( ( is_exists = ( EEXIST == errno ) ) )
					if ( -1 == ( fd = shm_open( m_name.c_str( ), O_RDWR, 0 ) ) )
						return; // perror( )
			} else {
				if ( -1 == ftruncate( fd, m_size ) ) 
					return; // perror( )
			}
		} else {
			fd = shm_open( m_name.c_str( ), O_RDWR, 0 );
			if ( -1 == fd ) 
				return; // perror( )
			// To known size
			struct stat buf = { };
			if ( -1 == fstat( fd, &buf ) ) 
				return; // perror( )
			m_size = buf.st_size;
		}

		m_buff = mmap( NULL, m_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );

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
		return GetMemPtr( ) == MAP_FAILED;
    }
    void* GetMemPtr() const {
		return m_buff;
    }
};
} // namespace Ipc
