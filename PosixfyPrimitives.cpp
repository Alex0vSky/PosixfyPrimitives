// PosixfyPrimitives.cpp - entry point // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)

int main(int argc, char **argv) {
#if ( defined( _DEBUG ) ) && ( defined( _WIN32 ) )
    HeapSetInformation( NULL, HeapEnableTerminationOnCorruption, NULL, NULL );
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	//_CrtSetBreakAlloc( Xxx );
#	ifdef new
#		error `new` has been redefined
#	endif
	new char[]{ "Goobay!" };
#endif // ( defined( _DEBUG ) ) & ( defined( _WIN32 ) )

	// for `boost::system::error_code::message` language
	setlocale( 0, "" );

	::testing::InitGoogleTest( &argc, argv );
	std::cout << "Running main() from Posixer.cpp" << std::endl;
	return RUN_ALL_TESTS( );
}
