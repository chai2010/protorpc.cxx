:: Copyright 2013 <chaishushan{AT}gmail.com>. All rights reserved.
:: Use of this source code is governed by a BSD-style
:: license that can be found in the LICENSE file.

@echo off
setlocal

cd %~dp0

rmdir /S/Q zz_build_win32_proj_mt_tmp_debug
rmdir /S/Q zz_build_win32_proj_mt_tmp_release
rmdir /S/Q zz_build_win64_proj_mt_tmp_debug
rmdir /S/Q zz_build_win64_proj_mt_tmp_release

del /Q protorpc-win32.lib
del /Q protorpc-win32-mt.lib
del /Q protorpc-win32-debug.lib
del /Q protorpc-win32-debug-mt.lib

del /Q protorpc-win64.lib
del /Q protorpc-win64-mt.lib
del /Q protorpc-win64-debug.lib
del /Q protorpc-win64-debug-mt.lib

del /Q protoc-win32.exe
del /Q rpctest-win32.exe
del /Q rpcserver-win32.exe
del /Q rpcclient-win32.exe
del /Q xmltest-win32.exe

del /Q protoc-win64.exe
del /Q rpctest-win64.exe
del /Q rpcserver-win64.exe
del /Q rpcclient-win64.exe
del /Q xmltest-win64.exe

