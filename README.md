# ofxWhisper

openFramworks addon for [OpenAI's Whisper](https://github.com/openai/whisper) using [whisper.cpp](https://github.com/ggerganov/whisper.cpp/)

Developed with openFramworks v0.11.2. Should work with newer versions as well.

This is a rough implementation, still WIP

## Install
install this addon in the default addons folder of your openFramworks install.
```
cd your_openframeworks_folder/addons/
git clone https://github.com/roymacdonald/ofxWhisper.git
```
## dependencies
This addon requires you to also have installed [ofxSampleRate](https://github.com/roymacdonald/ofxSampleRate). Please check its readme file to get it installed properly.

## building lib
Run the `build_lib.sh` script to download and compile for your platform.
This will download also the basic model. Look at [whisper.cpp](https://github.com/ggerganov/whisper.cpp/)' readme on how to download other models.

If you are not using macos make sure that before runinng the script  you modify the line that reads
`cp libwhisper.a ../libs/whisper_cpp/lib/osx/`

and change `osx` so it matches your platform.

```
cd your_openframeworks_folder/addons/ofxWhisper
.build_lib.sh
```

## Last but not least
Remember to update the example project using Project Generator


