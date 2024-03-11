// src/Posix/Event.h - independent process event facility
// @insp SO/linux-posix-equivalent-for-win32s-createevent-setevent-waitforsingleobject
#pragma once // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
namespace Intraprocess {
namespace detail {
class MutexEvent {
	pthread_mutex_t m_mutex;

	class Guard {
		MutexEvent *m_parent;

	public:
		explicit Guard(MutexEvent *parent) : 
			m_parent( parent ) 
		{
		   m_parent ->lock( );
		}
		~Guard() { 
			m_parent ->unlock( );
		}
	};

public:
    MutexEvent(MutexEvent const&) = delete; void operator=(MutexEvent const&) = delete;
	MutexEvent() {
		pthread_mutex_init( &m_mutex, nullptr );
	}
	~MutexEvent() {
		pthread_mutex_destroy( &m_mutex );
	}
	void lock() {
		pthread_mutex_lock( &m_mutex );
	}
	void unlock() {
		pthread_mutex_unlock( &m_mutex );
	}
	Guard scoped_guard() {
		return Guard( this );
	}
	operator pthread_mutex_t *() {
		return &m_mutex;
	}
};

class ControlBlock {
	struct Private{};
	MutexEvent m_mutex;
	bool m_value;

public:
	typedef std::shared_ptr< ControlBlock > controlBlock_t;
	ControlBlock(Private, bool initial_state) : 
		m_value( initial_state ) 
	{}
	static controlBlock_t create(bool initial_state = false) {
		return std::make_shared< ControlBlock >( Private(), initial_state );
	}
	void set() {
		m_mutex.scoped_guard( );
		m_value = ( true );
	}
	void reset() {
		m_mutex.scoped_guard( );
		m_value = ( false );
	}
	bool is_set() {
		m_mutex.scoped_guard( );
		return m_value;
	}
	auto scoped_guard() {
		return m_mutex.scoped_guard( );
	}
	auto &get_mutex() {
		return m_mutex;
	}
};
} // namespace detail
class CEvent {
	bool m_manual_reset;
	// Is `mutable` to keep methods signatures
	mutable EventHandle h_event;
	mutable detail::ControlBlock::controlBlock_t m_controlBlock;

public:
	CEvent(bool is_manual_reset, bool initial_state) :
		m_manual_reset( is_manual_reset )
		, m_controlBlock( detail::ControlBlock::create( initial_state ) )
	{}
	CEvent(const CEvent& other) :
		m_manual_reset( other.m_manual_reset )
		, m_controlBlock( other.m_controlBlock )
		, h_event( CTools::CopyHandle( other.h_event ) )
	{}

	const CEvent& operator = (const CEvent& other) {
		if ( this != &other ) {
			m_manual_reset = ( other.m_manual_reset );
			m_controlBlock = ( other.m_controlBlock );
			CTools::CloseAndInvalidateHandle( h_event );
			h_event = CTools::CopyHandle( other.h_event );
		}
		return *this;
	}
	~CEvent() {
		CTools::CloseAndInvalidateHandle( h_event );
	}
	void Set() {
		if ( !h_event ) return;
		m_controlBlock ->set( );
		if ( m_manual_reset )
			::pthread_cond_broadcast( h_event );
		else
			::pthread_cond_signal( h_event );
	}
	void Reset() {
		if ( !h_event ) return;
		m_controlBlock ->reset( );
	}
	bool Wait(unsigned timeout_milli=0) const {
		if ( !h_event ) return false;

		timespec abstime = CTools::MilliToAbsoluteTimespec( timeout_milli );

		int timedwait = 0;
		// Spurious wakeups
		while ( !m_controlBlock ->is_set( ) ) {
			auto guard = m_controlBlock ->scoped_guard( );
			timedwait = pthread_cond_timedwait( h_event, m_controlBlock ->get_mutex( ), &abstime );
			if ( ETIMEDOUT == timedwait )
				break;
		}
		if ( 0 == timedwait && !m_manual_reset ) 
			m_controlBlock ->reset( );

		return ( 0 == timedwait );
	}
	bool WaitInfinite() const {
		return Wait( INFINITE );
	}
	bool IsSet() const {
		return Wait( 0 );
	}
};
} // namespace Intraprocess
