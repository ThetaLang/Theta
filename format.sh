clang-format --style=file -i $(find . -wholename '**/*.cpp' -or -wholename '**/*.hpp' -or -wholename '**/*.h' | grep -v '/lib')
