// src/Posix/Event.h - event facility // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
// @insp SO/linux-posix-equivalent-for-win32s-createevent-setevent-waitforsingleobject
#pragma once // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
namespace IndependentProcess {
class CHandleEvent {
	pthread_cond_t h_event;
	pthread_condattr_t attr_;
	pthread_cond_t h_invalid_event;
	pthread_condattr_t invalid_attr_;
public:
	CHandleEvent() {
		::pthread_cond_init( &h_invalid_event, &invalid_attr_ );
		// The attribute and control block parameters of the condition variable will not be valid after destruction, but can be reinitialized by calling pthread_cond_init() or statically.
		::pthread_cond_destroy( &h_invalid_event );
	}

	operator bool() const {
		return true;
	}
};

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

class CEvent {
	bool is_manual_reset_;
	bool initial_state_;
	// Is `mutable` to keep methods signatures
	mutable bool signaled_;
	mutable pthread_cond_t h_event;
	mutable MutexEvent mutex_;

	bool *x;

	// @insp https://stackoverflow.com/questions/15024623/convert-milliseconds-to-timespec-for-gnu-port
	static void ms2ts(timespec *ts, unsigned long milli) {
		ts ->tv_sec = milli / 1000;
		ts ->tv_nsec = (milli % 1000) * 1000000;
	}
	// To avoid Integer overflow: abstime.tv_sec += adding.tv_sec; abstime.tv_nsec += adding.tv_nsec;
	static void safe_add(timespec *accum, timespec *src) {
		long sum_tv_sec; // decltype( timespec::tv_sec ) 
		if ( __builtin_saddl_overflow( accum ->tv_sec, src ->tv_sec, &sum_tv_sec ) ) {
			accum ->tv_sec = std::numeric_limits< long >::max( );
		} else {
			accum ->tv_sec = sum_tv_sec;
		}
		long sum_tv_nsec;
		if ( __builtin_saddl_overflow( accum ->tv_nsec, src ->tv_nsec, &sum_tv_nsec ) ) {
			accum ->tv_nsec = std::numeric_limits< long >::max( );
		} else {
			accum ->tv_nsec = sum_tv_nsec;
		}
	}

public:
	CEvent(bool is_manual_reset, bool initial_state) :
		is_manual_reset_( is_manual_reset )
		, initial_state_( initial_state )
		, signaled_( false )
		, x( &signaled_ )
	{
		pthread_cond_init( &h_event, nullptr );
		if ( initial_state_ )
			Set( );
	}
	CEvent(const CEvent& other) :
		is_manual_reset_( other.is_manual_reset_ )
		, initial_state_( other.initial_state_ )
		, signaled_( other.signaled_ )
		, x( other.x )
		, h_event( CTools::CopyHandle( other.h_event ) )
	{}

	const CEvent& operator = (const CEvent& other) {
		if ( this != &other ) {
			is_manual_reset_ = ( other.is_manual_reset_ );
			initial_state_ = ( other.initial_state_ );
			signaled_ = ( other.signaled_ );
			x = ( other.x );
			CTools::CloseAndInvalidateHandle( h_event );
			h_event = CTools::CopyHandle( other.h_event );
		}
		return *this;
	}
	~CEvent() {
		CTools::CloseAndInvalidateHandle( h_event );
	}
	void Set() {
//		if ( !h_event ) return;
		{
			auto scoped_guard = mutex_.scoped_guard( );
			signaled_ = true;
			if ( &signaled_ != x )
				*x = true;
		}
		if ( is_manual_reset_ )
			::pthread_cond_broadcast( &h_event );
		else
			::pthread_cond_signal( &h_event );
	}
	void Reset() {
//		if ( !h_event ) return;
		{
			auto scoped_guard = mutex_.scoped_guard( );
			signaled_ = false;
			if ( &signaled_ != x )
				*x = false;
		}
	}
	bool Wait(unsigned timeout_milli=0) const {
//		if ( !h_event ) return false;

		timespec abstime = { };
		if ( INFINITE == timeout_milli ) {
			abstime.tv_sec = std::numeric_limits< decltype( abstime.tv_sec ) >::max( );
			abstime.tv_nsec = std::numeric_limits< decltype( abstime.tv_nsec ) >::max( );
		} else {
			abstime.tv_sec = time( nullptr ); // clock_gettime( CLOCK_REALTIME, &abstime );
			timespec adding = { }; 
			ms2ts( &adding, timeout_milli );
			safe_add( &abstime, &adding ); //abstime.tv_sec += adding.tv_sec; abstime.tv_nsec += adding.tv_nsec;
		}

		// Success if not enter to waiting
		int timedwait = 0;
		{
			auto scoped_guard = mutex_.scoped_guard( );
			// Spurious wakeups
			while ( !*x ) {
				timedwait = pthread_cond_timedwait( &h_event, mutex_, &abstime );
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
} // namespace IndependentProcess
