echo "copy file to packet"
cd ..\..\..
copy /y .\libs\*.lib  .\build\windows\x64\ace_release_static_win10_x64_v7.0.2\libs\
xcopy .\include\*  .\build\windows\x64\ace_release_static_win10_x64_v7.0.2\include\ /s /e /y
copy /y .\docs\*  .\build\windows\x64\ace_release_static_win10_x64_v7.0.2\docs\
copy /y .\COPYING  .\build\windows\x64\ace_release_static_win10_x64_v7.0.2\
copy /y .\AUTHORS  .\build\windows\x64\ace_release_static_win10_x64_v7.0.2\
copy /y .\NEWS  .\build\windows\x64\ace_release_static_win10_x64_v7.0.2\
copy /y .\README  .\build\windows\x64\ace_release_static_win10_x64_v7.0.2\
copy /y .\THANKS  .\build\windows\x64\ace_release_static_win10_x64_v7.0.2\
copy /y .\VERSION.txt  .\build\windows\x64\ace_release_static_win10_x64_v7.0.2\
echo ok
pause