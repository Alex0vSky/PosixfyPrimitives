// src/Windows/Event.h - independent process event facility 
#pragma once // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
namespace IndependentProcess {
class CEvent {
	HANDLE h_event;

public:
	CEvent(bool is_manual_reset, bool initial_state) {
		h_event = CreateEventA( NULL,(BOOL)is_manual_reset,(BOOL)initial_state,NULL );
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
		if ( h_event )
			::SetEvent(h_event);
	}
	void Reset() {
		if ( h_event )
			::ResetEvent(h_event);
	}
	bool Wait(unsigned timeout_ms=0) const {
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
