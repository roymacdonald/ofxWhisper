#!/bin/bash

check_make_dir(){
	if [ ! -d "$1" ]; then
		mkdir "$1"
	fi
}


SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $SCRIPT_DIR	

git clone --depth 1  https://github.com/ggerganov/whisper.cpp.git

cd whisper.cpp


check_make_dir ../libs/whisper_cpp/models/

./models/download-ggml-model.sh base.en

make libwhisper.a	


cp libwhisper.a ../libs/whisper_cpp/lib/osx/
cp whisper.h ../libs/whisper_cpp/include/
cp ggml.h ../libs/whisper_cpp/include/
cp examples/common.h ../libs/whisper_cpp/include/
cp examples/common.cpp ../libs/whisper_cpp/src/
cp models/ggml-base.en.bin ../libs/whisper_cpp/models/

cd ..
rm -rf whisper.cpp