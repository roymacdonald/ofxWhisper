#include "ofxWhisper.h"
#include "ofxWhisperUtils.h"

//-----------------------------------------------------------------------------------------------
std::string ofxWhisper::to_timestamp(int64_t t) {
    int64_t sec = t/100;
    int64_t msec = t - sec*100;
    int64_t min = sec/60;
    sec = sec - min*60;
    
    char buf[32];
    snprintf(buf, sizeof(buf), "%02d:%02d.%03d", (int) min, (int) sec, (int) msec);
    
    return std::string(buf);
}

//-----------------------------------------------------------------------------------------------
ofxWhisper::ofxWhisper():
updatePeriod(0)
{
}
//-----------------------------------------------------------------------------------------------
ofxWhisper::~ofxWhisper(){
    wait_cond_var.notify_all();
    if(isThreadRunning()){
        waitForThread(true, 5000);
    }
    
    if(bIsSetup || ctx){
        whisper_print_timings(ctx);
        whisper_free(ctx);
    }
 
    
}

//-----------------------------------------------------------------------------------------------
void ofxWhisper::setup(const ofxWhisperSettings& _settings, int numInputChannels){
    this->settings = _settings;
    settings.model = ofToDataPath(settings.model, true);
    settings.keep_ms   = std::min(settings.keep_ms,   settings.step_ms);
    settings.length_ms = std::max(settings.length_ms, settings.step_ms);
    
    n_samples_step = (1e-3*settings.step_ms  )*WHISPER_SAMPLE_RATE;
    n_samples_len  = (1e-3*settings.length_ms)*WHISPER_SAMPLE_RATE;
    n_samples_keep = (1e-3*settings.keep_ms  )*WHISPER_SAMPLE_RATE;
    
    use_vad = n_samples_step <= 0; // sliding window mode uses VAD
    
    ofLogNotice(" ofxWhisper::setup") << "Use VAD: " << boolalpha << use_vad;
    
    n_new_line = !use_vad ? std::max(1, settings.length_ms / settings.step_ms - 1) : 1; // number of steps to print new line
    
    settings.no_timestamps  = !use_vad;
    settings.no_context    |= use_vad;
    settings.max_tokens     = 0;
    
    audio_input = make_unique<ofxWhisperBufferResampler>();
    
    audio_input->setup(numInputChannels, settings.length_ms *5);
    
    
    if (whisper_lang_id(settings.language.c_str()) == -1) {
        ofLogError("ofxWhisper::setup") << "error: unknown language " << settings.language;
        bIsSetup = false;
        return;
    }
    
    ctx = whisper_init_from_file(settings.model.c_str());
    
    if (!whisper_is_multilingual(ctx)) {
        if (settings.language != "en" || settings.translate) {
            settings.language = "en";
            settings.translate = false;
            ofLogWarning ("ofxWhisper::setup") << " WARNING: model is not multilingual, ignoring language and translation options";
        }
    }
    

    
    ofLogNotice("ofxWhisper::setup") << getInfoString();
    
    
    ofLogNotice("ofxWhisper") << "[Start speaking]";

    _lastUpdateStartTime = std::chrono::system_clock::now();
    _lastUpdateEndTime = std::chrono::system_clock::now();

    bIsSetup = true;
    
    startThread();
}
//-----------------------------------------------------------------------------------------------
bool ofxWhisper::setupAudioInput(const ofxWhisperSettings& whisperSettings, size_t deviceIndex, int sampleRate, int bufferSize, ofSoundDevice::Api api){
    soundStream = make_unique<ofSoundStream> ();
    
    auto devices = soundStream->getDeviceList(api);
    if ( deviceIndex < devices.size()) {
        int i = deviceIndex;
        
        setup(whisperSettings, devices[i].inputChannels);
                
        ofSoundStreamSettings settings;
        
        settings.setInDevice(devices[i]);
        settings.setInListener(this);

        settings.numInputChannels = devices[i].inputChannels;
        
        //You can pass 0 as the value for inSampleRate and then the value will be the highest one available in the chosen device
        settings.sampleRate = (sampleRate > 0)?sampleRate:getMaxValue(devices[i].sampleRates);
        settings.numBuffers = 2;
        settings.bufferSize = bufferSize;
        
        return soundStream->setup(settings);
    }
    return false;
}

//-----------------------------------------------------------------------------------------------
ofRectangle ofxWhisper::draw(){
  
    
    ofRectangle r;
    if(audio_input){
        r = audio_input->draw({10, 20, 30, 200});
    }

    stringstream ss;
    ss << "Update Period: " << updatePeriod.load() ;
    

    float x = r.x;
    float y = r.getMaxY() + 20;
    
    r.growToInclude(bf.getBoundingBox(ss.str(), x,y));
    
    ofDrawBitmapStringHighlight(ss.str(), x,y);
    
    return  r;

    
}

//-----------------------------------------------------------------------------------------------
std::string ofxWhisper::getInfoString(){
    std::stringstream ss;
    ss << "ofxWhisper info:\n";
    ss <<  "processing " << n_samples_step << "\n";
    ss <<"  step : " << float(n_samples_step)/WHISPER_SAMPLE_RATE  << " sec.\n";
    ss <<"  len : " << float(n_samples_len )/WHISPER_SAMPLE_RATE << " sec.\n";
    ss <<"  keep : " << float(n_samples_keep)/WHISPER_SAMPLE_RATE << " sec.";
    ss <<"  num threads : " << settings.n_threads << "\n";
    ss <<"  language : " << settings.language.c_str() << "\n";
    ss <<"  task : " << (settings.translate ? "translate" : "transcribe") << "\n";
    ss <<"  timestamps : " << (settings.no_timestamps ? "false" : "true") << "\n";
    
    
    ss << "n_samples_step : " << n_samples_step << "\n";
    ss << "n_samples_len : " << n_samples_len << "\n";
    ss << "n_samples_keep : " << n_samples_keep << "\n";
    
    if (!use_vad) {
        ss << "n_new_line : " <<  n_new_line << "\n";
        ss << "no_context : " << settings.no_context << "\n";
    } else {
        ss << "using VAD, will transcribe on speech activity\n";
    }
    
    
    
    return ss.str();
}

//-----------------------------------------------------------------------------------------------
bool ofxWhisper::_isReadyToUpdate(){
    if(audio_input){
        return audio_input->hasBufferedMs(settings.step_ms);
    }
    return false;
}
//-----------------------------------------------------------------------------------------------
void ofxWhisper::threadedFunction() {
    
    
    auto wait_predicate = std::bind(std::mem_fn(&ofxWhisper::_isReadyToUpdate), this);
    
    while(isThreadRunning()){
        
        auto lastUpdateDuration = _lastUpdateEndTime - _lastUpdateStartTime;
        updatePeriod = std::chrono::microseconds(lastUpdateDuration).count();
        auto stepDuration = std::chrono::milliseconds(settings.step_ms);
        if(lastUpdateDuration >= stepDuration ){
            ofLogNotice()<< "lastUpdateDuration >= stepDuration : " ;//<< lastUpdateDuration << " >= " << stepDuration;
            while(true){
                if(_isReadyToUpdate()){
                    break;
                }
                sleep(2);
            }
             
            update();
            
        }else{
            
            auto timePoint = std::chrono::system_clock::now() + (stepDuration - lastUpdateDuration);
            std::unique_lock<std::mutex> uLock(wait_mutex);
            if (wait_cond_var.wait_until(uLock, timePoint, wait_predicate))
            {
                update();
            }
            else // timeout occured, conditions are not fulfilled
            {
                ofLogError("ofxWhisper::threadedFunction") << "wait_until return false.";
                // e.g. do some error handling
            }
        }

  
    }
}

//-----------------------------------------------------------------------------------------------
void ofxWhisper::_updateNoVAD(){
    
    size_t n_samples = WHISPER_SAMPLE_RATE * settings.step_ms / 1000.0f;
    
    buffer_new.allocate(n_samples, audio_input->getNumChannels());
    buffer_new.setSampleRate(WHISPER_SAMPLE_RATE);
    
    if(audio_input->get(buffer_new)){
        if(buffer_new.getNumFrames()>0){ 
    const int n_samples_new = buffer_new.getNumFrames();

    const int n_samples_take = std::min((int) buffer_old.getNumFrames(), std::max(0, n_samples_keep + n_samples_len - n_samples_new));

    buffer.allocate(n_samples_new + n_samples_take, audio_input->getNumChannels());
        buffer.setSampleRate(WHISPER_SAMPLE_RATE);

        if(buffer_old.size()> 0){
            buffer_old.copyTo(buffer, n_samples_take, buffer.getNumChannels(), buffer_old.getNumFrames() - n_samples_take);
        }
        
            buffer.append(buffer_new);
            buffer_old = buffer;

        }
    }
}


//-----------------------------------------------------------------------------------------------
void ofxWhisper::_updateVAD(){
////    const auto t_now  = ofGetElapsedTimeMillis();
////    const auto t_diff = (t_now - t_last);
//    if(audio_input->get(settings.step_ms, pcmf32_new)){
////    audio_input->get(2000, pcmf32_new);
//
//    if (::vad_simple(pcmf32_new, WHISPER_SAMPLE_RATE, 1000, settings.vad_thold, settings.freq_thold, false)) {
//        audio_input->get(settings.length_ms, pcmf32);
////    } else {
////        sleep(100);
////
////        return;
//    }
//    }
////    t_last = t_now;
}

//-----------------------------------------------------------------------------------------------
void ofxWhisper::update(){
    _lastUpdateStartTime = std::chrono::system_clock::now();
    
//    ofLogNotice("ofxWhisper::update");
    // process new audio
    if(audio_input == nullptr) return;
    
    if(use_vad){
        _updateVAD();
    }else{
        _updateNoVAD();
    }
    _runInference();
    _processResults();
    _lastUpdateEndTime = std::chrono::system_clock::now();
}


//-----------------------------------------------------------------------------------------------
void ofxWhisper::_runInference(){
        
        whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
        
        wparams.print_progress   = false;
        wparams.print_special    = settings.print_special;
        wparams.print_realtime   = false;
        wparams.print_timestamps = !settings.no_timestamps;
        wparams.translate        = settings.translate;
//        wparams.no_context       = true;
        wparams.single_segment   = !use_vad;
        wparams.max_tokens       = settings.max_tokens;
        wparams.language         = settings.language.c_str();
        wparams.n_threads        = settings.n_threads;
        
        wparams.audio_ctx        = settings.audio_ctx;
        wparams.speed_up         = settings.speed_up;
        
        // disable temperature fallback
        wparams.temperature_inc  = -1.0f;
        
        
        wparams.temperature_inc  = settings.no_fallback ? 0.0f : wparams.temperature_inc;

        wparams.prompt_tokens    = settings.no_context ? nullptr : prompt_tokens.data();
        wparams.prompt_n_tokens  = settings.no_context ? 0       : prompt_tokens.size();
        
    
        ofSoundBuffer mono;
        buffer.getChannel(mono, 0);
    
        if (whisper_full(ctx, wparams, mono.getBuffer().data(), mono.getBuffer().size()) != 0) {
            ofLogError("ofxWhisper::audioIn") << "failed to process audio";
            return ;
        }

}



//-----------------------------------------------------------------------------------------------
void ofxWhisper::_processResults()
{
    {
//        if (!use_vad) {
//            //                printf("\33[2K\r");
//            //
//            //                // print long empty line to clear the previous line
//            //                printf("%s", std::string(100, ' ').c_str());
//            //
//            //                printf("\33[2K\r");
//        } else {
//            const int64_t t1 = (t_last - t_start)/1000000;
//            const int64_t t0 = std::max(0.0, t1 - pcmf32.size()*1000.0/WHISPER_SAMPLE_RATE);
//
//            stringstream ss;
//
//
//            ss << "\n### Transcription " << n_iter <<" START | t0 = " << (int) t0 << " ms | t1 = " << (int) t1 << " ms\n\n";
//            textChannel.send(ss.str());
//        }
        stringstream ss;
        
        const int n_segments = whisper_full_n_segments(ctx);
        for (int i = 0; i < n_segments; ++i) {
            string text (whisper_full_get_segment_text(ctx, i));
            if (settings.no_timestamps == false) {
                
                const int64_t t0 = whisper_full_get_segment_t0(ctx, i);
                const int64_t t1 = whisper_full_get_segment_t1(ctx, i);
                
                ss << "[" << to_timestamp(t0) << " --> "<< to_timestamp(t1) << "]" ;
            }
            ss << text;
            if (!settings.no_timestamps && whisper_full_get_segment_speaker_turn_next(ctx, i)) {
                ss << " [SPEAKER_TURN]";
            }
            if(i < n_segments -1){
                ss << "\n";
            }
        }
        if (use_vad){
            ss << "\n### Transcription " << n_iter << " END\n";
        }
        
        textChannel.send(ss.str());
    }
    
    ++n_iter;
    
    if (!use_vad && (n_iter % n_new_line) == 0) {
        printf("\n");
        
        // keep part of the audio for next iteration to try to mitigate word boundary issues
        
        buffer_old.allocate(n_samples_keep, buffer.getNumChannels());
        buffer.copyTo(buffer_old, n_samples_keep, buffer.getNumChannels(), buffer.getNumFrames() - n_samples_keep);
        
        // Add tokens of the last full length segment as the prompt
        if (!settings.no_context) {
            prompt_tokens.clear();
            
            const int n_segments = whisper_full_n_segments(ctx);
            for (int i = 0; i < n_segments; ++i) {
                const int token_count = whisper_full_n_tokens(ctx, i);
                for (int j = 0; j < token_count; ++j) {
                    prompt_tokens.push_back(whisper_full_get_token_id(ctx, i, j));
                }
            }
        }
    }
}

#ifdef USING_OFX_SOUND_OBJECTS

//-----------------------------------------------------------------------------------------------
void ofxWhisper::audioIn(ofSoundBuffer &input){

    if(inputObject!=nullptr) {
        ofLogWarning("ofxWhisper::audioIn") << "ofxWhisper is set as audio input, yet some other ofxSoundObject is connected to this one.";
    }
    ofxSoundInput::audioIn(input);
}

//-----------------------------------------------------------------------------------------------
void ofxWhisper::audioOut(ofSoundBuffer &output){
    if(inputObject!=nullptr) {
        ofxSoundObject::audioOut(output);
    }else{
        ofxSoundInput::audioOut(output);
    }
}

//-----------------------------------------------------------------------------------------------
void ofxWhisper::process(ofSoundBuffer &input, ofSoundBuffer &output) {
    if(audio_input) audio_input->push(input);
    output = input;
}
#else
//-----------------------------------------------------------------------------------------------
void ofxWhisper::audioIn(ofSoundBuffer &input){
    if(audio_input) audio_input->push(input);
}
#endif
