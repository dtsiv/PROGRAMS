; -- Example1.iss --
; Demonstrates copying 3 files and creating an icon.

; SEE THE DOCUMENTATION FOR DETAILS ON CREATING .ISS SCRIPT FILES!

[Setup]
AppName=Radar Indicator
AppVersion=1.0
VersionInfoVersion=1.0.0.0
AppPublisher=JSC Tristan, Inc.
AppPublisherURL=http://3stan.ru
AppCopyright=Copyright (C) 2019 JSC Tristan, Inc.
WizardStyle=modern
DefaultDirName={commonpf32}\Radar Indicator
DefaultGroupName=Radar Indicator
UninstallDisplayIcon={app}\RadarIndicator.exe
Compression=lzma2
SolidCompression=yes
OutputDir=..\distributive
SourceDir=dependencies
OutputBaseFilename=RadarIndicatorSetup
PrivilegesRequired=admin

[Files]
Source: "RadarIndicator.exe"; DestDir: "{app}"
Source: "intl.dll";           DestDir: "{app}"
Source: "libeay32.dll";       DestDir: "{app}"
Source: "libpq.dll";          DestDir: "{app}"
Source: "msvcr120.dll";       DestDir: "{app}"
Source: "opengl32sw.dll";     DestDir: "{app}"
Source: "ssleay32.dll";       DestDir: "{app}"
Source: "RadarIndicator.xml"; DestDir: "{app}"

[Icons]
Name: "{group}\Radar Indicator"; Filename: "{app}\RadarIndicator.exe"
