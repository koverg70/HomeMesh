set PATH=%PATH%;c:\utils\Electronics\WinAVR-20100110\bin;c:\utils\Electronics\gnuwin32\bin;c:/java/jdk1.7_x86/bin/../jre/bin/client;c:/java/jdk1.7_x86/bin/../jre/bin;c:/java/jdk1.7_x86/bin/../jre/lib/i386;C:\Python27\;C:\Python27\Scripts;C:\ProgramData\Oracle\Java\javapath;C:\WINDOWS\system32;C:\WINDOWS;C:\WINDOWS\System32\Wbem;C:\WINDOWS\System32\WindowsPowerShell\v1.0\;C:\Program Files (x86)\Brackets\command;C:\Program Files\TortoiseHg\;C:\Program Files (x86)\NVIDIA Corporation\PhysX\Common;C:\Program Files\Microsoft SQL Server\110\Tools\Binn\;C:\Program Files (x86)\nodejs\;C:\Users\koverg\AppData\Roaming\npm;C:\java\eclipse-cpp
echo #define NODE_ID 10 > node_id.h
cd Debug

make all && c:\utils\Electronics\arduino-1.0\hardware\tools\avr\bin\avrdude -pm328p -carduino "-P\\.\COM3" -b115200 -Uflash:w:HomeMesh.hex:a && start C:\utils\teraterm-4.78\ttermpro.exe /C=3 /BAUD=115200

cd..

