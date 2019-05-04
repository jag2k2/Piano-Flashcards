/*----------Function Prototypes-------*/
long DLLEXPORT MIDIReadClose(long hmidiin);
void CALLBACK MIDICallBack(HMIDIIN handle, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
long DLLEXPORT MIDIReadOpen(HMIDIIN hmidiin, unsigned long deviceID, LVUserEventRef *UserEvent);
int __stdcall DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) ;
void PopulateStringHandle(LStrHandle lvStringHandle,char* stringData);
/*------------------------------------*/
