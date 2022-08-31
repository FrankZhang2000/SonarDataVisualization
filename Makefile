CXXFLAGS_1 = -I/usr/local/MATLAB/R2021a/extern/include -L/usr/local/MATLAB/R2021a/bin/glnxa64 \
	-lglut -lGL -lGLU -lmat -lmx -Wl,-rpath /usr/local/MATLAB/R2021a/bin/glnxa64 -O2

CXXFLAGS_2 = -I/usr/local/MATLAB/R2021a/extern/include -L/usr/local/MATLAB/R2021a/bin/glnxa64 \
	-lmat -lmx -Wl,-rpath /usr/local/MATLAB/R2021a/bin/glnxa64 -O2

CFLAGS = -I/usr/local/include/freetype2 -lfreetype -lm -O2

TOP_DIR := $(shell pwd)
BUILD_DIR ?= $(TOP_DIR)/build
SRC_DIR := $(TOP_DIR)/src

.PHONY: all
all: ${BUILD_DIR}/visualization ${BUILD_DIR}/datagenerator ${BUILD_DIR}/getmetrics ${BUILD_DIR}/genbitmap

${BUILD_DIR}/visualization: ${SRC_DIR}/visualization.cpp ${SRC_DIR}/classifier.cpp ${SRC_DIR}/fontformatter.cpp \
		${SRC_DIR}/preprocessor.cpp ${SRC_DIR}/objwriter.cpp ${SRC_DIR}/settings.h ${SRC_DIR}/utils.h \
		${SRC_DIR}/classifier.h ${SRC_DIR}/fontformatter.h ${SRC_DIR}/preprocessor.h ${SRC_DIR}/objwriter.h
	g++ -std=c++17 ${SRC_DIR}/visualization.cpp ${SRC_DIR}/classifier.cpp ${SRC_DIR}/fontformatter.cpp \
			${SRC_DIR}/preprocessor.cpp ${SRC_DIR}/objwriter.cpp ${CXXFLAGS_1} -o ${BUILD_DIR}/visualization

${BUILD_DIR}/datagenerator: ${SRC_DIR}/datagenerator.cpp ${SRC_DIR}/joinobject.cpp ${SRC_DIR}/preprocessor.cpp \
		${SRC_DIR}/objwriter.cpp ${SRC_DIR}/joinobject.h ${SRC_DIR}/preprocessor.h ${SRC_DIR}/objwriter.h \
		${SRC_DIR}/settings.h ${SRC_DIR}/utils.h 
	g++ -std=c++17 ${SRC_DIR}/datagenerator.cpp ${SRC_DIR}/joinobject.cpp ${SRC_DIR}/preprocessor.cpp \
			${SRC_DIR}/objwriter.cpp ${CXXFLAGS_2} -o ${BUILD_DIR}/datagenerator

${BUILD_DIR}/getmetrics: ${SRC_DIR}/getmetrics.cpp ${SRC_DIR}/preprocessor.cpp ${SRC_DIR}/classifier.cpp \
		${SRC_DIR}/preprocessor.h ${SRC_DIR}/classifier.h ${SRC_DIR}/settings.h ${SRC_DIR}/utils.h
	g++ -std=c++17 ${SRC_DIR}/getmetrics.cpp ${SRC_DIR}/preprocessor.cpp ${SRC_DIR}/classifier.cpp \
			${CXXFLAGS_2} -o ${BUILD_DIR}/getmetrics

${BUILD_DIR}/genbitmap: ${SRC_DIR}/genbitmap.c ${SRC_DIR}/charset.h
	gcc ${SRC_DIR}/genbitmap.c ${CFLAGS} -o ${BUILD_DIR}/genbitmap

.PHONY: clean
clean:
	rm build/*