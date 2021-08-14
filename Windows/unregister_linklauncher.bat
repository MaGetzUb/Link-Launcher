

Reg.exe delete "HKLM\SOFTWARE\Link Launcher" /f
Reg.exe delete "HKLM\SOFTWARE\RegisteredApplications" /v "Link Launcher" /f
Reg.exe delete "HKLM\Software\Classes\LinkLauncherURL" /f

Reg.exe delete "HKCU\Environment" /v "LINK_LAUNCHER" /f