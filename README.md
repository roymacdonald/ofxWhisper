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


## Last but not least
Remember to update the example project using Project Generator


#