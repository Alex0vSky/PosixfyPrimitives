// PosixfyPrimitives.cpp - entry point // Copyright 2024 Alex0vSky (https://github.com/Alex0vSky)

struct Foo {
	bool flag;
	bool *x;
	bool _; Foo() : _{}
		, flag( false )
		, x( &flag )
	{}
	Foo(const Foo& other) : _{}
		, x( other.x )
	{}
	void set() {
		flag = true;
		if ( &flag != x )
			*x = true;
	}
	void reset() {
		flag = false;
		if ( &flag != x )
			*x = false;
	}
};

int main(int argc, char **argv) {

	Foo event1;
	event1.set( );
	event1.reset( );
	Foo event2;
	event2 = event1;
	event2 .set( );
	__nop( );

#if ( defined( _DEBUG ) ) & ( defined( _WIN32 ) )
    HeapSetInformation( NULL, HeapEnableTerminationOnCorruption, NULL, NULL );
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	//_CrtSetBreakAlloc( 161 );
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
