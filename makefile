.PHONY: tags

all: Makefile
	make -f Makefile
Makefile: CMakeLists.txt
        cmake -DSlicer3_DIR=${HOME}/LIBRARIES/SLICER/Slicer3-build -DCMAKE_BUILD_TYPE=Debug
tags:
	ctags --langmap=c++:+.txx  -R .  ${HOME}/LIBRARIES/SLICER/Slicer3 ${HOME}/LIBRARIES/SLICER/Slicer3-build ${HOME}/LIBRARIES/SLICER/Slicer3-lib
