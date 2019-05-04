/*------------------------------------
MIDI Input DLL
------------------------------------*/ 
#include <cvirte.h>
#include <windows.h>
#include <ansi_c.h>
#include <mmsystem.h>
#include "extcode.h"  
#include "MIDIDLL.h"


#define STRING_LENGHT 80

/*----------DLL GLOBALS---------------*/
	MIDIHDR			midiHdr;
	unsigned long	err;
	LVUserEventRef  *UserEventGlobal;
	DWORD			globalDWParam1;
	unsigned long 	*parameter0;
	unsigned char 	SysXBuffer[256]; 
	unsigned char 	SysXFlag = 0; 
/*------------------------------------*/



/*------------------------------------
	Closes the midi resource
------------------------------------*/ 
long DLLEXPORT MIDIReadClose(long hmidiin)
{
	SysXFlag |= 0x80;   
	midiInReset((HMIDIIN)hmidiin);	
	midiInUnprepareHeader((HMIDIIN)hmidiin, &midiHdr, sizeof(MIDIHDR));
	while ((err = midiInClose((HMIDIIN)hmidiin)) == MIDIERR_STILLPLAYING) Sleep(0);
	return 0;
}

/*------------------------------------
	Populates a LabVIEW string with the 
	MIDI data. It is necessary to use
	the CINTOOL LabVIEW functions to
	create a LabVIEW string that creates a
	string using LabVIEW's string format
------------------------------------*/ 
void PopulateStringHandle(LStrHandle lvStringHandle,char* stringData)
{
	//Empties the buffer
	memset(LStrBuf(*lvStringHandle),'\0',STRING_LENGHT);
	
	//Fills the string buffer with stringData
	sprintf((char*)LStrBuf(*lvStringHandle),"%s",stringData);
	
	//Informs the LabVIEW string handle about the size of the size
	LStrLen(*lvStringHandle)=strlen(stringData);
	
	return;
}

/*------------------------------------
	Proceses the MIDI data, and sends
	a message to the registered
	User Event
------------------------------------*/ 
	
void CALLBACK MIDICallBack(HMIDIIN handle, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)

{

	LPMIDIHDR		lpMIDIHeader;
	unsigned char 	*	ptr;
	//TCHAR			buffer[80];
	unsigned char 	bytes;
	//LStrHandle newStringHandle; 

	globalDWParam1 = dwParam1;
	/* Determine why Windows called me */
	switch (uMsg)
	{
		/* Received some regular MIDI message */
		case MIM_DATA:
		{
			/* Send message to the registered LabVIEW User Event */
			PostLVUserEvent(*UserEventGlobal, (void *)&globalDWParam1);
			break;
		}

		/* Received all or part of some System Exclusive message */

		case MIM_LONGDATA:
		{

		/* If this application is ready to close down, then don't midiInAddBuffer() again */
			if (!(SysXFlag & 0x80))
			{
				/*	Assign address of MIDIHDR to a LPMIDIHDR variable. Makes it easier to access the
			  		field that contains the pointer to our block of MIDI events */
				lpMIDIHeader = (LPMIDIHDR)dwParam1;

				/* Get address of the MIDI event that caused this call */
				ptr = (unsigned char *)(lpMIDIHeader->lpData);

				/* Is this the first block of System Exclusive bytes? */
				if (!SysXFlag)
				{
					/* Indicate we've begun handling a particular System Exclusive message */
					SysXFlag |= 0x01;
				}
				/* Is this the last block (ie, the end of System Exclusive byte is here in the buffer)? */

				if (*(ptr + (lpMIDIHeader->dwBytesRecorded - 1)) == 0xF7)
				{
					/* Indicate we're done handling this particular System Exclusive message */
					SysXFlag &= (~0x01);
				}
				/* Display the bytes -- 16 per line */
				bytes = 16;
				midiInAddBuffer(handle, lpMIDIHeader, sizeof(MIDIHDR));
		}		
		break;
		}
	}
}

	
/*------------------------------------
	Opens the MIDI device
------------------------------------*/ 
long DLLEXPORT MIDIReadOpen(HMIDIIN hmidiin, unsigned long deviceID, LVUserEventRef *UserEvent)
{

	UserEventGlobal = UserEvent;
	SysXFlag = 0;
	
	/* Open default MIDI In device */

	if (!(err = midiInOpen(&hmidiin, deviceID, (DWORD)MIDICallBack, 0, CALLBACK_FUNCTION)))
	{
		/* Store pointer to our input buffer for System Exclusive messages in MIDIHDR */
		midiHdr.lpData = (LPBYTE)&SysXBuffer[0];
		/* Store its size in the MIDIHDR */
		midiHdr.dwBufferLength = sizeof(SysXBuffer);
		/* Flags must be set to 0 */
		midiHdr.dwFlags = 0;

		/* Prepare the buffer and MIDIHDR */
		err = midiInPrepareHeader(hmidiin, &midiHdr, sizeof(MIDIHDR));
		if (!err)
		{
			/* Queue MIDI input buffer */
			err = midiInAddBuffer(hmidiin, &midiHdr, sizeof(MIDIHDR));
			if (!err)
			{
				/* Start recording Midi */
				err = midiInStart(hmidiin);
				if (!err)
				{
				}

			}
		}

	else

	{
	   /* future error handling code */
	}



	return(err);
	}
	return 0;
}


/*------------------------------------
	Standard Main function for
	DLL
------------------------------------*/ 
int __stdcall DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			if (InitCVIRTE (hinstDLL, 0, 0) == 0)
				return 0;	  /* out of memory */
			break;
		case DLL_PROCESS_DETACH:
			CloseCVIRTE ();
			break;
	}
	
	return 1;
}

int __stdcall DllEntryPoint (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	/* Included for compatibility with Borland */

	return DllMain (hinstDLL, fdwReason, lpvReserved);
}
