#include <windows.h>

int main(int argc, char* argv[])
{
	if( argc != 3 ) return 0;

	SHELLEXECUTEINFO sei;
	ZeroMemory( &sei, sizeof( sei ) );
	sei.lpFile = argv[1];
	sei.lpParameters = argv[2];
	sei.cbSize = sizeof( sei );
	ShellExecuteEx( &sei );

	return 0;
}