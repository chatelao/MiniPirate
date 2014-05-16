#include "Strings_PGM_MEM.h"



//-----------------------------------------------------------------------------------------------------------------
void printProgramString( prog_char * str, Print & target )
	{
	static char program_string_copy_buffer[100];  
	strcpy_P(program_string_copy_buffer, (str) );
	target.print( program_string_copy_buffer );
//	target.print( "cheese!" );
	}