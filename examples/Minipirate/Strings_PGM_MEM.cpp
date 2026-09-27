#include "Strings_PGM_MEM.h"

//-----------------------------------------------------------------------------------------------------------------
#if defined(HAS_PGMSPACE) && !defined(ESP8266) && !defined(ESP32)
void printProgramString (const char * str PROGMEM, Print & target)
{
	static char program_string_copy_buffer[100];  
	strcpy_P(program_string_copy_buffer, (str) );
	target.print( program_string_copy_buffer );
//	target.print( "cheese!" );
}
#endif
