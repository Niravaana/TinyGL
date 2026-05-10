# TinyGL — Claude Instructions

## Commit Messages

Never add `Co-Authored-By:` trailers. No Claude attribution in any commit, ever.

## Building

Use the Visual Studio Developer Command Prompt environment. To build from a shell:

```bash
cmd /c "\"C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat\" && msbuild src\OpenGL\OpenGL.vcxproj /p:Configuration=Debug /p:Platform=x64"
```

This is the confirmed working build command. Do not try to discover MSBuild, do not guess paths, do not use PowerShell. Use the command above exactly.

VS Developer Command Prompt shortcut location (for reference):
`C:\ProgramData\Microsoft\Windows\Start Menu\Programs\Visual Studio 2022\Visual Studio Tools`
