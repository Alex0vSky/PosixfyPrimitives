// src\Log.h - with formatting warnings, thread safe if printf/wprintf is safe, 
// using tuple to heavy, a buffer need to avoid multi evaluate __VA_ARGS__ values
// @insp SO/how-do-i-expand-a-tuple-into-variadic-template-functions-arguments
#pragma once // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
#ifndef A0S_cppcheck__ // Cant pass '// cppcheck-suppress cstyleCast' to macro
#	define LOG(fmt, ...) \
		do {																						\
__pragma( warning( push ) )																			\
__pragma( warning( disable: 4906 ) )																\
__pragma( warning( disable: 4774 ) )																\
			using namespace std::chrono;															\
			constexpr bool bWchar = std::is_convertible_v< decltype( fmt ), const wchar_t * >;		\
			constexpr bool bChar = std::is_convertible_v< decltype( fmt ), const char * >;			\
			static_assert( bWchar || bChar, "Print only wchar_t or char strings");					\
			auto A0S_LOG_now = system_clock::now( );												\
			auto A0S_LOG_ms = duration_cast<milliseconds>(A0S_LOG_now.time_since_epoch()) % 1000;	\
			auto A0S_LOG_timer = system_clock::to_time_t(A0S_LOG_now);								\
			std::tm A0S_LOG_bt; localtime_s(&A0S_LOG_bt, &A0S_LOG_timer);							\
			std::ostringstream ossA;																\
			ossA << std::put_time(&A0S_LOG_bt, "%T");												\
			ossA << '.' << std::setfill('0') << std::setw(3) << A0S_LOG_ms.count() << " | ";		\
			std::string dateTimeA = ossA.str( );													\
			std::wstring dateTimeW( dateTimeA.begin(), dateTimeA.end() );							\
			char bufA[ 1024 + 1 ] = { };															\
			wchar_t bufW[ 1024 + 1 ] = { };															\
			bChar																					\
				? (																					\
					snprintf( bufA, 1024, (const char *)( fmt ), __VA_ARGS__ )						\
					, dateTimeA += bufA																\
					, dateTimeA += "\n"																\
					, printf( "%s", dateTimeA.c_str( ) )											\
					, (void)0																		\
					)																				\
				: (																					\
					_snwprintf_s( bufW, 1024, (const wchar_t *)( fmt ), __VA_ARGS__ )				\
					, dateTimeW += bufW																\
					, dateTimeW += L"\n"															\
					, wprintf( L"%s", dateTimeW.c_str( ) )											\
					, (void)0																		\
					)																				\
				;																					\
__pragma( warning( pop ) )																			\
		} while( false )
#endif // A0S_cppcheck__
