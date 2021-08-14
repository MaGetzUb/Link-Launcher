@echo off

pushd .
cd ..
setx LINK_LAUNCHER %cd%
popd



Reg.exe add "HKLM\SOFTWARE\Link Launcher\Capabilities" /v "ApplicationDescription" /t REG_SZ /d "Launch Links to different browsers" /f
Reg.exe add "HKLM\SOFTWARE\Link Launcher\Capabilities" /v "ApplicationName" /t REG_SZ /d "Link Launcher" /f
Reg.exe add "HKLM\SOFTWARE\Link Launcher\Capabilities" /v "ApplicationIcon" /t REG_SZ /d "%LINK_LAUNCHER%\LinkLauncher.exe,0" /f
Reg.exe add "HKLM\SOFTWARE\Link Launcher\Capabilities\FileAssociations" /v ".htm" /t REG_SZ /d "LinkLauncherURL" /f
Reg.exe add "HKLM\SOFTWARE\Link Launcher\Capabilities\FileAssociations" /v ".html" /t REG_SZ /d "LinkLauncherURL" /f
Reg.exe add "HKLM\SOFTWARE\Link Launcher\Capabilities\FileAssociations" /v ".shtml" /t REG_SZ /d "LinkLauncherURL" /f
Reg.exe add "HKLM\SOFTWARE\Link Launcher\Capabilities\FileAssociations" /v ".xht" /t REG_SZ /d "LinkLauncherURL" /f
Reg.exe add "HKLM\SOFTWARE\Link Launcher\Capabilities\FileAssociations" /v ".xhtml" /t REG_SZ /d "LinkLauncherURL" /f
Reg.exe add "HKLM\SOFTWARE\Link Launcher\Capabilities\URLAssociations" /v "ftp" /t REG_SZ /d "LinkLauncherURL" /f
Reg.exe add "HKLM\SOFTWARE\Link Launcher\Capabilities\URLAssociations" /v "http" /t REG_SZ /d "LinkLauncherURL" /f
Reg.exe add "HKLM\SOFTWARE\Link Launcher\Capabilities\URLAssociations" /v "https" /t REG_SZ /d "LinkLauncherURL" /f
REM ;Register as default program
Reg.exe add "HKLM\SOFTWARE\RegisteredApplications" /v "Link Launcher" /t REG_SZ /d "Software\Link Launcher\Capabilities" /f
Reg.exe add "HKLM\Software\Classes\LinkLauncherURL" /ve /t REG_SZ /d "Web Browser Document" /f
Reg.exe add "HKLM\Software\Classes\LinkLauncherURL" /v "FriendlyTypeName" /t REG_SZ /d "Web Browser Document" /f
Reg.exe add "HKLM\Software\Classes\LinkLauncherURL\shell\open\command" /ve /t REG_SZ /d "\"%LINK_LAUNCHER%\LinkLauncher.exe\" \"%%1\"" /f