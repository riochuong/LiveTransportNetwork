
## LIVE TRANSPORT NETWORK MONITOR PROJECT

#### CONAN 2.0 SETUP
It's important to add this line to CMakeLists.txt
`include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)`

#### VSCODE SETUP
Check the content of `conan_toolchain.cmake` in build folder and add proper include path 
via `C/C++: edit configurations` option using search via `Ctrl + Shift + P`

Example content:
```
list(PREPEND CMAKE_INCLUDE_PATH "/home/chuongdao/.conan2/p/boost99f33a443c700/p/include" "/home/chuongdao/.conan2/p/zlib4be8ddd7aa752/p/include" "/home/chuongdao/.conan2/p/bzip26d48265eb59fb/p/include" "/home/chuongdao/.conan2/p/libbab546f21710147/p/include")

```



