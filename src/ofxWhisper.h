
#pragma once
#include "ofThread.h"
#include "ofThreadChannel.h"
#include "common.h"
#include "whisper.h"
#include "ofSoundBaseTypes.h"
#include "ofUtils.h"
#include "ofLog.h"
#include "ofxWhisperBufferedSoundInput.h"

struct ofxWhisperSettings {
    //n_threads "number of threads to use during computation\n", ;
    int32_t n_threads  = std::min(4, (int32_t) std::thread::hardware_concurrency());
    
    int32_t step_ms    = 3000;     //step_ms "audio step size in milliseconds\n",             ;
    
    int32_t length_ms  = 10000;     //length_ms "audio length in milliseconds\n",                ;
    
    int32_t keep_ms    = 200;     //keep_ms "audio to keep from previous step in ms\n",      ;
    
    int32_t capture_id = -1;    //capture_id "capture device ID\n",                           ;
    
    int32_t max_tokens = 32;    //max_tokens "maximum number of tokens per audio chunk\n",    ;
    
    int32_t audio_ctx  = 0;    //audio_ctx "audio context size (0 - all)\n",                ;
    
    float vad_thold    = 0.6f; //"voice activity detection threshold\n"
    
    float freq_thold   = 100.0f; //"high-pass frequency cutoff\n"
    
    bool speed_up      = false; //"speed up audio by x2 (reduced accuracy)\n",
    bool translate     = false; //"translate from source language to english\n",
    bool print_special = false; //"print special tokens\n",
    bool no_context    = true; //"keep context between audio chunks\n",
    bool no_timestamps = false;
    
    std::string language  = "en"; //"spoken language\n",
    std::string model     = "models/ggml-base.en.bin"; //"model path\n",
    std::string fname_out; //"text output file name\n",
};


class ofxWhisper: public ofThread{
    
    ofxWhisperSettings settings;
    int n_samples_step = 0;
    int n_samples_len = 0;
    int n_samples_keep = 0;
    bool use_vad = false;
    int n_new_line = 1;
    bool bIsSetup = false;
    
    
    uint64_t t_last;
    uint64_t t_start;
    struct whisper_context * ctx ;
    int n_iter = 0;

    std::vector<whisper_token> prompt_tokens;

public:
    virtual ~ofxWhisper();
    
    unique_ptr<ofxWhisperBufferedSoundInput> audio_input = nullptr;
    

    ofThreadChannel<std::string> textChannel;    
    
    
    
    
    std::string getInfoString();
    
    void setup(ofxWhisperSettings _settings);

    std::string to_timestamp(int64_t t) ;
    
    virtual void threadedFunction() override;
    
    void update();
    
    
};
