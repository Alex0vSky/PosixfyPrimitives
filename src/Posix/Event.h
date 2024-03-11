// src/Posix/Event.h - independent process event facility // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
// @insp SO/linux-posix-equivalent-for-win32s-createevent-setevent-waitforsingleobject
#pragma once // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
namespace Intraprocess {

namespace detail {
class MutexEvent {
	pthread_mutex_t mutex_;

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
		pthread_mutex_init( &mutex_, nullptr );
	}
	~MutexEvent() {
		pthread_mutex_destroy( &mutex_ );
	}
	void lock() {
		pthread_mutex_lock( &mutex_ );
	}
	void unlock() {
		pthread_mutex_unlock( &mutex_ );
	}
	operator pthread_mutex_t *() {
		return &mutex_;
	}
	Guard scoped_guard() {
		return Guard( this );
	}
};

class ControlBlock {
	struct Private{};
	detail::MutexEvent m_mutex;
	// or std::atomic_bool without mutex, must faster
	bool m_value;

public:
	typedef std::shared_ptr< ControlBlock > controlBlock_t;
	//struct controlBlock_t : public std::shared_ptr< ControlBlock > {
	//	operator bool() = delete;
	//};
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
};

//// Avoid pointer bool
//ControlBlock::controlBlock_t::operator bool(detail::ControlBlock::controlBlock_t const&) = delete;

} // namespace detail

class CEvent {
	bool is_manual_reset_;
	bool initial_state_;
	// Is `mutable` to keep methods signatures
	mutable bool signaled_;
	mutable EventHandle h_event;
	mutable detail::MutexEvent mutex_;

	detail::ControlBlock::controlBlock_t m_controlBlock;
	bool *x;

public:
	CEvent(bool is_manual_reset, bool initial_state) :
		is_manual_reset_( is_manual_reset )
		, initial_state_( initial_state )
		, signaled_( false )
		, x( &signaled_ )
		, m_controlBlock( detail::ControlBlock::create( initial_state ) )
	{
		if ( initial_state_ )
			Set( );
	}
	CEvent(const CEvent& other) :
		is_manual_reset_( other.is_manual_reset_ )
		, initial_state_( other.initial_state_ )
		, signaled_( other.signaled_ )
		, x( other.x )
		, m_controlBlock( other.m_controlBlock )
		, h_event( CTools::CopyHandle( other.h_event ) )
	{}

	const CEvent& operator = (const CEvent& other) {
		if ( this != &other ) {
			is_manual_reset_ = ( other.is_manual_reset_ );
			initial_state_ = ( other.initial_state_ );
			signaled_ = ( other.signaled_ );
			x = ( other.x );
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
		{
			auto scoped_guard = mutex_.scoped_guard( );
			signaled_ = true;
			if ( &signaled_ != x )
				*x = true;
		}
		if ( is_manual_reset_ )
			::pthread_cond_broadcast( h_event );
		else
			::pthread_cond_signal( h_event );
	}
	void Reset() {
		if ( !h_event ) return;
		{
			auto scoped_guard = mutex_.scoped_guard( );
			signaled_ = false;
			if ( &signaled_ != x )
				*x = false;
		}
	}
	bool Wait(unsigned timeout_milli=0) const {
		if ( !h_event ) return false;

		timespec abstime = CTools::MilliToAbsoluteTimespec( timeout_milli );

		// Success if not enter to waiting
		int timedwait = 0;
		{
			auto scoped_guard = mutex_.scoped_guard( );
			// Spurious wakeups
			while ( !*x ) {
				timedwait = pthread_cond_timedwait( h_event, mutex_, &abstime );
				if ( ETIMEDOUT == timedwait )
					break;
			}
		}
		if ( 0 == timedwait && !is_manual_reset_ ) {
			auto scoped_guard = mutex_.scoped_guard( );
			signaled_ = false;
			if ( &signaled_ != x )
				*x = false;
		}
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
