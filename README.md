# psychosomatic.exe
GDI malware made by Itzsten made for educational purposes only.

It's written in C, and the bootloader is written entirely in x86 assembly (please don't do that).

# Tips
I recommend having a dark background, hence effects will become more visible.

# Requirements
Windows 8.1+ x64 (x86 coming out soon, possibly, no WOW64 support if it does though)
English language! (Won't work without it, due to how the SYSTEM account name will be different)

# Destructive payloads
- Overwrites LogonUI, try dragging the messages boxes once on the screen!
- Overwrites the Master Boot Record (Will not work on VMWARE for unknown causes, please use a copy of the bootloader presented in this repository for emulation in QEMU instead)
- Corrupts registry keys
- Encrypts files
- UAC bypass (levels 1-3)
- Hooking NtQuerySystemInformation, NtTerminateProcess, NtResumeThread & more for persistence once in the system. (Will not show up in Task Manager, Process Hacker etcetera)

# Warning
This malware is dedicated for entertainment and educational purposes only. I am not responsible for any damages caused by using this malware, and by running it, you agree that you've acknowledged the consequences that will follow.

Please do not try executing any "psychosomatic leaks" or builders. These all contain Discord Token grabbers or IP loggers.

# Note
Enjoy!
