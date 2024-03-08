// src/Posix/Event.h - event facility // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
#pragma once // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
namespace IndependentProcess {
class CEvent {
	pthread_cond_t h_event;
	pthread_condattr_t m_attr;

public:
	CEvent(bool is_manual_reset, bool initial_state) noexcept(false) {
		//h_event = pthread_cond_init( NULL,(BOOL)is_manual_reset,(BOOL)initial_state,NULL );
		h_event = PTHREAD_COND_INITIALIZER;
		if ( pthread_cond_init( &h_event, &m_attr ) ) {
			throw std::exception( "pthread_cond_init" );
		}
	}
	CEvent(const CEvent& other) {
		h_event = CTools::CopyHandle( other.h_event );
	}
	const CEvent& operator = (const CEvent& other) {
		if ( this != &other ) {
			CTools::CloseAndInvalidateHandle(h_event);
			h_event = CTools::CopyHandle(other.h_event);
		}
		return *this;
	}
	~CEvent() {
		CTools::CloseAndInvalidateHandle(h_event);
	}
	void Set() {
		//pthread_cond_signal or pthread_cond_broadcast
		if ( h_event )
			::SetEvent(h_event);
	}
	void Reset() {
		if ( h_event )
			::ResetEvent(h_event);
	}
	bool Wait(unsigned timeout_ms=0) const {
		//pthread_cond_wait or pthread_cond_timedwait
		return h_event ? WaitForSingleObject(h_event,timeout_ms) == WAIT_OBJECT_0 : false;
	}
	bool WaitInfinite() const {
		return Wait(INFINITE);
	}
	bool IsSet() const {
		return Wait(0);
	}
};
} // namespace IndependentProcess
