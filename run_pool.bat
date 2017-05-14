@echo off
REG ADD "HKCU\Software\Microsoft\Windows\Windows Error Reporting" /f /v Disabled /t REG_DWORD /d 1 >nul
REG ADD "HKCU\Software\Microsoft\Windows\Windows Error Reporting" /f /v DontShowUI /t REG_DWORD /d 1 >nul
REG ADD "HKLM\Software\Microsoft\Windows\Windows Error Reporting" /f /v Disabled /t REG_DWORD /d 1 >nul
REG ADD "HKLM\Software\Microsoft\Windows\Windows Error Reporting" /f /v DontShowUI /t REG_DWORD /d 1 >nul
@echo Windows Error Reporting UI was Disabled !!!
@echo -------------------------------------------
@echo !!!!! Change to your NXS address below and delete this line !!!!!!
:begin
nexus_cpuminer.exe nexusminingpool.com 9550 2SSbxVqakuEQeTLk8cgGKeWCiVrsyJtFMrvKxJDmuY3nRAmr2uJ %1 %2 %3 
goto begin