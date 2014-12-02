//This contains the code for debugging! When trace is on, will automatically append to the trace file.
//Made for CS3307 Final Project
#include <stdio.h>
#include <stdarg.h>
#include <fstream>	//for i/o
#include "debug.h" 	//for debug stuff!

//PURPOSE: Prints the message. It also prints the message to the excution trace file ONLY if the execution trace is on!
//PARAMETER: const char *fmt for you know, the message, any arguments...
//RETURNS: r, total number of characters written. writing error would give negative I think. Also, check my note below.
//NOTE: Credits go to cplusplus.com/reference/cstdio/vprintf for the framework of this coding... Go check there for specifics.
int bank_printf(bool traceSignal, const char *fmt, ...)
{
	//This section here basically prints the formmated data.
	va_list ap;
	int r = 0;
/*
	va_start(ap, fmt);
	r = vfprintf(stderr, fmt, ap);
	va_end(ap);
*/

	//If the TRACE is on...    
	if(traceSignal == true)
    	{
		//Open up an appendable trace.txt
		FILE * pFile;
		pFile = fopen ("trace.txt","a");

		//Write to that file.
        	va_start(ap, fmt);
        	vfprintf(pFile, fmt, ap);
       		va_end(ap);

		//Close that file.
        	fclose(pFile);
    }
    
    return r;
}


