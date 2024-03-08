// tests/testEvent.cpp - test posixfy event // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)
#include "CTools.h"
#ifdef _WIN32
#	include "Windows/SharedMem.h"
#else
#	include "Posix/SharedMem.h"
#endif
using CSharedMem = Ipc::CSharedMem;
static const unsigned size = 4096;

namespace testSharedMemory_ { 

TEST(event_set, wait0) {
	bool already_exists;
	bool open_existing;

	CSharedMem *sharedMem1 = CSharedMem::Create( "asd", size , &already_exists );
	CSharedMem *sharedMem2 = CSharedMem::Create( "asd", size , &already_exists );
	CSharedMem::Free( sharedMem1 );
	CSharedMem::Free( sharedMem2 );

	EXPECT_TRUE( already_exists );
}

} // namespace testSharedMemory_ 
