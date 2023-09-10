
#pragma once
#include "ofThread.h"
#include "ofThreadChannel.h"
#include "common.h"
#include "whisper.h"
#include "ofSoundBaseTypes.h"
#include "ofUtils.h"
#include "ofLog.h"
#include "ofxWhisperBufferedSoundInput.h"

//#ifdef USING_OFX_SOUND_OBJECTS
//#include "ofxSoundObjects.h"
//#endif
//#include "waveformDraw.h"

#include <chrono>

struct ofxWhisperSettings {
    //n_threads "number of threads to use during computation\n", ;
    int32_t n_threads  = std::min(4, (int32_t) std::thread::hardware_concurrency());
    
    int32_t step_ms    = 3000;     //step_ms "audio step size in milliseconds\n",             ;
    
    int32_t length_ms  = 5000;     //length_ms "audio length in milliseconds\n",                ;
    
    int32_t keep_ms    = 200;     //keep_ms "audio to keep from previous step in ms\n",      ;
    
    int32_t capture_id = -1;    //capture_id "capture device ID\n",                           ;
    
    int32_t max_tokens = 32;    //max_tokens "maximum number of tokens per audio chunk\n",    ;
    
    int32_t audio_ctx  = 0;    //audio_ctx "audio context size (0 - all)\n",                ;
    
    float vad_thold    = 0.2f; //"voice activity detection threshold\n"
    
    float freq_thold   = 100.0f; //"high-pass frequency cutoff\n"
    
    bool speed_up      = false; //"speed up audio by x2 (reduced accuracy)\n",
    bool translate     = false; //"translate from source language to english\n",
    bool no_fallback   = false;
    bool print_special = false; //"print special tokens\n",
    bool no_context    = true; //"keep context between audio chunks\n",
    bool no_timestamps = false;
    bool tinydiarize   = false;
    
    std::string language  = "en"; //"spoken language\n",
    std::string model     = "models/ggml-base.en.bin"; //"model path\n",
    std::string fname_out; //"text output file name\n",
};


class ofxWhisper: public ofThread
//#ifdef USING_OFX_SOUND_OBJECTS
//, public ofxSoundObject
//#endif

{
    
    ofxWhisperSettings settings;
    int n_samples_step = 0;
    int n_samples_len = 0;
    int n_samples_keep = 0;
    bool use_vad = false;
    int n_new_line = 1;
    bool bIsSetup = false;
    
    
//    uint64_t t_last;
//    uint64_t t_start;
    struct whisper_context * ctx ;
    int n_iter = 0;

    std::vector<whisper_token> prompt_tokens;

    std::mutex wait_mutex;
    std::condition_variable wait_cond_var;
    
    void _runInference();
    void _processResults();
    
//    std::chrono::time_point<std::chrono::system_clock> _lastUpdateTime;
    
    
    bool _isReadToUpdate();
    void _updateNoVAD();
    void _updateVAD();
    
    
//    waveformDraw wave, wave_old;
//    circularBufferWaveformDraw wave_new;
//
    ofBitmapFont bf;
    uint64_t lastUpdate = 0;
public:
    
    std::atomic<uint64_t> updatePeriod;
    
    void toggleWaveBypass();
    virtual ~ofxWhisper();
    ofxWhisper();
    
    unique_ptr<ofxWhisperBufferedSoundInput> audio_input = nullptr;
    

    ofRectangle draw();
    ofThreadChannel<std::string> textChannel;    
    
//#ifdef USING_OFX_SOUND_OBJECTS
//    virtual void process(ofSoundBuffer &input, ofSoundBuffer &output) override;
//#endif
    
    
//    std::vector<float> pcmf32 ;
//    std::vector<float> pcmf32_old;
//    std::vector<float> pcmf32_new;
//
    ofSoundBuffer buffer, buffer_old, buffer_new;
    
    
    std::string getInfoString();
    
    void setup(ofxWhisperSettings _settings, int inputDeviceIndex , int inSampleRate, int bufferSize = 256 , ofSoundDevice::Api api = ofSoundDevice::Api::UNSPECIFIED);

    std::string to_timestamp(int64_t t) ;
    
    virtual void threadedFunction() override;
    
    void update();
    
    
};
