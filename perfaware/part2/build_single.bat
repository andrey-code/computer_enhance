@echo off

IF NOT EXIST build mkdir build
pushd build

where /q cl && (
  call cl /std:c++20 -Zi -W4 -EHsc -nologo ..\%1 -Fe%~n1_dm.exe
  call cl /std:c++20 -arch:AVX2 -O2 -Zi -W4 -EHsc -nologo ..\%1 -Fe%~n1_rm.exe
)

where /q clang++ && (
  call clang++ -g -std=c++20 -Wall -fuse-ld=lld ..\%1 -o %~n1_dc.exe
  call clang++ -mavx2 -std=c++20 -O3 -g -Wall -fuse-ld=lld ..\%1 -o %~n1_rc.exe
)

popd
