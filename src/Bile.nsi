# $Id: Bile.nsi,v 1.1 2010/09/01 10:38:25 ken Exp $
# :tabSize=4:indentSize=4:folding=indent:
OutFile "setup.exe"
InstallDir "$PROGRAMFILES\Kaia\Bile"

LicenseData LICENSE.txt
LicenseForceSelection checkbox

Page license
Page directory
Page components
Page instfiles
UninstPage uninstConfirm
UninstPage instFiles

Section "Program"
	SetOutPath $INSTDIR
	File "Bile.exe"
	WriteUninstaller "$INSTDIR\Uninst.exe"
SectionEnd


Section "Uninstall"
	Delete "$INSTDIR\Uninst.exe"
	Delete "$INSTDIR\Bile.exe"
	RMDir "$INSTDIR"
SectionEnd

