
@echo off
rem This file builds the DLL used by the test-case using the Mono C# 4.0 compiler.
dmcs -target:library qtmonotests.cs
