# MiniDumpPlugin

**Note**: This plugin has been integrated into x64dbg as the [minidump](https://help.x64dbg.com/en/latest/commands/memory-operations/minidump.html) command since 2022-10-10.

Simple [x64dbg](https://x64dbg.com) plugin to save the current state in a full minidump. Created for [dumpulator](https://github.com/mrexodia/dumpulator).

**Download the latest binaries [here](https://github.com/mrexodia/MiniDumpPlugin/releases).**

## Building

From a Visual Studio command prompt:

```
cmake -B build64 -A x64
cmake --build build64 --config Release
```

You will get `build64\MiniDump.sln` that you can open in Visual Studio.

To build a 32-bit plugin:

```
cmake -B build32 -A Win32
cmake --build build32 --config Release
```

Alternatively you can open this folder in Visual Studio/CLion/Qt Creator.

