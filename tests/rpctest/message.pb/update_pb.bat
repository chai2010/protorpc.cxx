
@rem gen cxx code
..\..\..\bin\protoc.exe --cxx_out=. arith.proto
..\..\..\bin\protoc.exe --cxx_out=. echo.proto

@rem gen go code
..\..\..\bin\protoc.exe --go_out=. arith.proto
..\..\..\bin\protoc.exe --go_out=. echo.proto
