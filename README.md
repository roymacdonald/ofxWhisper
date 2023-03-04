# ofxWhisper

openFramworks addon for [OpenAI's Whisper](https://github.com/openai/whisper) using [whisper.cpp](https://github.com/ggerganov/whisper.cpp/)

Developed with openFramworks v0.11.2. Should work with newer versions as well.

## building lib

It already comes with the x64 macos compiled lib, if it is your platform you can just compile and run the example.

Otherwise run the build_lib.sh script to download and compile for your platform.
make sure that before runinng it you modify the line that reads
```
cp libwhisper.a ../libs/whisper_cpp/lib/osx/```
and change `osx` so it matches your platform.