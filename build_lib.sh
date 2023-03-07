#!/bin/bash

CYAN='\033[1;36m'
RED='\033[1;31m'
YELL='\033[1;33m'
NC='\033[0m'

check_make_dir(){
	if [ ! -d "$1" ]; then
		mkdir "$1"
	fi
}


SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $SCRIPT_DIR	



echo -e "${YELL}Cloning whisper.cpp repository${NC}"

git clone --depth 1  https://github.com/ggerganov/whisper.cpp.git

cd whisper.cpp

check_make_dir ../libs/whisper_cpp/models/


echo -e "${YELL}Downloading base.en model${NC}"

./models/download-ggml-model.sh base.en


echo -e "${YELL}Building libwhisper${NC}"

if [[ "$OSTYPE" =~ ^msys ]]; then

 	if ! command -v cmake &>/dev/null; then
        echo -e "${RED}     You need to install CMake in order to compile whisper.cpp.     ${NC}"
		echo -e "${RED}     Download the installer from https://cmake.org/download/      ${NC}"
		echo -e "${RED}     Make sure you use the MSI installer rather than the zip      ${NC}"
		echo -e "${RED}     and choose \"Add CMake to the system PATH for all users\"      ${NC}"
		echo -e "${RED}     Exiting now!      ${NC}"
 
        exit 1
    fi

	cmake -S . -B ./build -A x64 -DCMAKE_BUILD_TYPE=Release -DWHISPER_SUPPORT_SDL2=OFF -DBUILD_SHARED_LIBS=OFF -DWHISPER_BUILD_EXAMPLES=OFF -DWHISPER_BUILD_TESTS=OFF -DWHISPER_ALL_WARNINGS=OFF
  	cd ./build 
  	/c/Program\ Files/Microsoft\ Visual\ Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe ALL_BUILD.vcxproj -t:build -p:configuration=Release -p:platform=x64
  	cd ..
else
make libwhisper.a	

fi


if [ $? -ne 0 ];then
	echo -e "${YELL}BUilding libwhisper FAILED!. Exiting${NC}"
	exit 1
fi

echo -e "${YELL}Copying files${NC}"

if [[ "$OSTYPE" =~ ^darwin ]]; then
	cp libwhisper.a ../libs/whisper_cpp/lib/osx/
fi

if [[ "$OSTYPE" =~ ^linux ]]; then
	if [[ "$HOSTTYPE" =~ ^x86_64 ]]; then
		cp libwhisper.a ../libs/whisper_cpp/lib/linux64/
	else
		cp libwhisper.a ../libs/whisper_cpp/lib/linux/
	fi
fi

if [[ "$OSTYPE" =~ ^msys ]]; then
	cp build/Release/whisper.lib ../libs/whisper_cpp/lib/vs/
fi	

cp whisper.h ../libs/whisper_cpp/include/
cp ggml.h ../libs/whisper_cpp/include/
cp models/ggml-base.en.bin ../libs/whisper_cpp/models/
cp models/ggml-base.en.bin ../example/bin/data/models/



echo -e "${YELL}Removing whisper.cpp downloaded files${NC}"
cd ..
rm -rf whisper.cpp

echo -e "${YELL}Install success.${NC}"