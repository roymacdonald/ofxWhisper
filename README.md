# ofxWhisper

openFramworks addon for [OpenAI's Whisper](https://github.com/openai/whisper) using [whisper.cpp](https://github.com/ggerganov/whisper.cpp/)

Developed with openFramworks v0.12.0 Should work with newer versions as well as v0.11.2


## Install
install this addon in the default addons folder of your openFramworks install.
```
cd your_openframeworks_folder/addons/
git clone https://github.com/roymacdonald/ofxWhisper.git
```
## dependencies
This addon requires you to also have installed [ofxSampleRate](https://github.com/roymacdonald/ofxSampleRate). Please check its readme file to get it installed properly.


## building lib

### Macos

This addon already includes the library compiled for Macos so you dont need to build it again.
It works on both, Intel and Apple CPUs.

For Apple processors (M1 and M2) it was built with support for CoreML which gives it a very noticeable processing speed increase. For this you will need to download additionaly the coreML model using the script in `libs/whisper_cpp/models/download-coreml-model.sh`


### Win/Linux

Run the `build_lib.sh` script to download and compile for your platform.
This will download also the basic model. Look at [whisper.cpp](https://github.com/ggerganov/whisper.cpp/)' readme on how to download other models.

If you are not using macos make sure that before runinng the script  you modify the line that reads
`cp libwhisper.a ../libs/whisper_cpp/lib/osx/`

and change `osx` so it matches your platform.

```
cd your_openframeworks_folder/addons/ofxWhisper
.build_lib.sh
```

### Models

It is very important that you download the necesary models. 

For Linux/Macos use the following script 

`libs/whisper_cpp/models/download-ggml-model.sh`

for windows use:
`libs/whisper_cpp/models/download-ggml-model.cmd`

Either of these scripts need to be called with the model name as argument. like

`libs/whisper_cpp/models/download-ggml-model.sh base.en` which will download the base model only for english language.


For more info check [here](https://github.com/ggerganov/whisper.cpp/blob/master/models/README.md)

#### M1/M2 apple computers
For these you will need to download additionaly the coreML model using the script in `libs/whisper_cpp/models/
download-coreml-model.sh`. You need to pass as argument the same you did for the regular model.

For example:

`libs/whisper_cpp/models/download-coreml-model.sh base.en`.

## Examples

There are 3 examples.

* example_simple
* example_ofxSoundObjects_AudioInput* example_ofxSoundObjects_AudioPlayer

For the last two you will need [ofxSoundObjects](https://github.com/roymacdonald/ofxSoundObjects/) and [ofxAudioFile](https://github.com/npisanti/ofxAudioFile) installed.

### example_simple
This will open a sound input (microphone) on your computer and process it with whisper. The results will be drawn on screen.
Make sure you choose the correct sound device.


### example_ofxSoundObjects_AudioInput
Uses ofxSoundObjects. The way it works is the same as example_simple but it connects trhough a chain of ofxSoundObjects. This allows you to either process the sound input before or after passing trhough whisper, using ofxSoundObjects. Check ofxSoundObjects examples to see how to work with it.


### example_ofxSoundObjects_AudioPlayer

Uses ofxSoundObjects. Loads an audiofile, plays it and proceeses it with whisper. 



## Last but not least
Remember to update the examples project using Project Generator

