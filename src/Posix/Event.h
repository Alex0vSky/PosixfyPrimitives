// src/Posix/Event.h - independent process event facility // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
// @insp SO/linux-posix-equivalent-for-win32s-createevent-setevent-waitforsingleobject
#pragma once // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
namespace Intraprocess {

namespace detail {
class MutexEvent {
	pthread_mutex_t m_mutex;

	class Guard {
		MutexEvent *m_parent;

	public:
		explicit Guard(MutexEvent *parent): m_parent( parent ) {
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
};

class ControlBlock {
	struct Private{};
	detail::MutexEvent m_mutex;
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
	bool isSet() const {
		return m_value;
	}
	Guard scoped_guard() {
		return Guard( this );
	}
};
} // namespace detail

class CEvent {
	bool is_manual_reset_;
	bool initial_state_;
	// Is `mutable` to keep methods signatures
	mutable EventHandle h_event;
	mutable detail::MutexEvent mutex_;

	detail::ControlBlock::controlBlock_t m_controlBlock;

public:
	CEvent(bool is_manual_reset, bool initial_state) :
		is_manual_reset_( is_manual_reset )
		, initial_state_( initial_state )
		, m_controlBlock( detail::ControlBlock::create( initial_state ) )
	{
		if ( initial_state_ )
			Set( );
	}
	CEvent(const CEvent& other) :
		is_manual_reset_( other.is_manual_reset_ )
		, initial_state_( other.initial_state_ )
		, m_controlBlock( other.m_controlBlock )
		, h_event( CTools::CopyHandle( other.h_event ) )
	{}

	const CEvent& operator = (const CEvent& other) {
		if ( this != &other ) {
			is_manual_reset_ = ( other.is_manual_reset_ );
			initial_state_ = ( other.initial_state_ );
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
		if ( is_manual_reset_ )
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

		// Success if not enter to waiting
		int timedwait = 0;
		{
			auto scoped_guard = mutex_.scoped_guard( );
			// Spurious wakeups
			while ( ! m_controlBlock ->isSet( ) ) {
				timedwait = pthread_cond_timedwait( h_event, mutex_, &abstime );
				if ( ETIMEDOUT == timedwait )
					break;
			}
		}
		if ( 0 == timedwait && !is_manual_reset_ ) 
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
