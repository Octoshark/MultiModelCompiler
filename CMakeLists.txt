cmake_minimum_required( VERSION 3.21 FATAL_ERROR )

project( MultiModelCompiler )
add_executable( MultiModelCompiler src/main.cpp )
set_target_properties(
	MultiModelCompiler PROPERTIES
	CXX_STANDARD 23 # This is necessary because std::format is gated behind the "latest" option
)