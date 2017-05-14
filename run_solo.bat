@echo off
REG ADD "HKCU\Software\Microsoft\Windows\Windows Error Reporting" /f /v Disabled /t REG_DWORD /d 1 >nul
REG ADD "HKCU\Software\Microsoft\Windows\Windows Error Reporting" /f /v DontShowUI /t REG_DWORD /d 1 >nul
REG ADD "HKLM\Software\Microsoft\Windows\Windows Error Reporting" /f /v Disabled /t REG_DWORD /d 1 >nul
REG ADD "HKLM\Software\Microsoft\Windows\Windows Error Reporting" /f /v DontShowUI /t REG_DWORD /d 1 >nul
@echo Windows Error Reporting UI was Disabled !!!
@echo -------------------------------------------
set NXS_WALLET_ADDRESS=localhost
:begin
nexus_cpuminer.exe %NXS_WALLET_ADDRESS% 9325 "" %1 %2 %3
goto begin