#include <filesystem>
#include <string_view>
#include <array>
#include <iostream>
#include <format>
#include <fstream>
#include <iomanip>

#include <Windows.h>

template <typename... Args>
void print( const std::string& format_str, Args&&... args ) {
	const std::string str = std::format( format_str, std::forward<Args>( args )... );
	std::fwrite( str.data(), 1, str.size(), stdout );
}

#define ARG_HELP "--help"
#define ARG_STUDIOMDLPATH "-studiomdlpath"
#define ARG_GAME "-game"
#define ARG_INPUTFILE "-input"

void print_help() {
	print( "Usage: MultiModelCompiler.exe {} [path] {} [path] {} [path]\n", ARG_STUDIOMDLPATH, ARG_GAME, ARG_INPUTFILE );
	print( "\t{}\t\tShow this message\n", ARG_HELP );
	print( "\t{}\tPath to studiomdl.exe\n", ARG_STUDIOMDLPATH );
	print( "\t{}\t\tPath to game folder\n", ARG_GAME );
	print( "\t{}\t\tPath to text file containing list of models\n", ARG_INPUTFILE );
}

bool isWhitespace ( char c ) {
	return ( c == ' ' || c == '\t' || c == '\n' || c == '\r' );
}

int main( int argc, char *argv[] ) {
	std::filesystem::path studiomdlAbsPath;
	std::filesystem::path gamePath;
	std::filesystem::path inputFile;

	std::vector<std::filesystem::path> qcPaths;

	if ( argc <= 1 || std::string_view( argv[1] ) == "--help" ) {
		print_help();
		return 0;
	}

	auto getNextArg = [&]( std::size_t currentArg ) -> std::string_view {
		if ( currentArg + 1 >= argc ) {
			return {};
		}

		return argv[ currentArg + 1 ];
	};

	for ( int i = 1; i < argc; ++i ) {
		
		std::string_view arg = argv[i];

		if ( arg == ARG_STUDIOMDLPATH ) {
			studiomdlAbsPath = std::filesystem::absolute( getNextArg( i++ ) );

			if ( studiomdlAbsPath.empty() ) {
				print( "{} missing path!\n", ARG_STUDIOMDLPATH );
				return EXIT_FAILURE;
			}
		}
		else if ( arg == ARG_HELP ) {
			continue;
		}
		else if ( arg == ARG_GAME ) {
			gamePath = std::filesystem::absolute( getNextArg( i++ ) );
			if ( gamePath.empty() ) {
				print( "{} missing path!\n", ARG_GAME );
				return EXIT_FAILURE;
			}
		}
		else if ( arg == ARG_INPUTFILE ) {
			inputFile = std::filesystem::absolute( getNextArg( i++ ) );
			if ( inputFile.empty() ) {
				print( "{} missing path!\n", ARG_INPUTFILE );
				return EXIT_FAILURE;
			}
		}
		else {
			print( "Unknown argument {}!\n", arg );
		}
	}

	if ( !std::filesystem::exists( inputFile ) ) {
		print( "Failed to find input file!\n" );
		return EXIT_FAILURE;
	}

	if ( !std::filesystem::exists( studiomdlAbsPath ) ) {
		print( "Failed to find studiomdl!\n" );
		return EXIT_FAILURE;
	}

	std::vector<char> fileData( std::filesystem::file_size( inputFile ) );
	std::ifstream( inputFile, std::ifstream::binary ).read( fileData.data(), fileData.size() );

	auto getStringEnd = [&fileData]( std::size_t start ) -> std::size_t {
		for ( std::size_t i = start; i < fileData.size(); ++i ) {
			if ( isWhitespace( fileData[i] ) ) {
				return i;
			}
		}

		return fileData.size();
	};

	for ( std::size_t i = 0; i < fileData.size(); ++i ) {
		if ( isWhitespace( fileData[i] ) ) {
			continue;
		}

		const std::size_t stringEnd = getStringEnd( i );
		qcPaths.emplace_back( std::string_view( &fileData[i], stringEnd - i ) );
		i = stringEnd;
	}

	print( "Processing files:\n" );

	for ( auto& QC : qcPaths ) {
		print( "{}\n", QC.generic_string() );
	}

	for ( std::size_t i = 0; i < qcPaths.size(); ++i )
	{
		std::wstring params = std::format( L"-nop4 {}", qcPaths[i].c_str() );
		ShellExecuteW( nullptr, L"open", studiomdlAbsPath.c_str(), params.c_str(), nullptr, SW_SHOWDEFAULT );
	}
}