#include "ofxWhisper.h"

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
ofxWhisper::~ofxWhisper(){
    if(isThreadRunning()){
        waitForThread(true, 5000);
    }
    if(bIsSetup || ctx){
        whisper_print_timings(ctx);
        whisper_free(ctx);
    }
 
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
    
    if (!use_vad) {
        ss << "n_new_line : " <<  n_new_line << "\n";
        ss << "no_context : " << settings.no_context;
    } else {
        ss << "using VAD, will transcribe on speech activity\n";
    }
    return ss.str();
}

//-----------------------------------------------------------------------------------------------
void ofxWhisper::setup(ofxWhisperSettings _settings){
    this->settings = _settings;
    settings.model = ofToDataPath(settings.model, true);
    settings.keep_ms   = std::min(settings.keep_ms,   settings.step_ms);
    settings.length_ms = std::max(settings.length_ms, settings.step_ms);
    
    n_samples_step = (1e-3*settings.step_ms  )*WHISPER_SAMPLE_RATE;
    n_samples_len  = (1e-3*settings.length_ms)*WHISPER_SAMPLE_RATE;
    n_samples_keep = (1e-3*settings.keep_ms  )*WHISPER_SAMPLE_RATE;
    
    //use_vad = n_samples_step <= 0; // sliding window mode uses VAD
    use_vad = true;
    
    n_new_line = !use_vad ? std::max(1, settings.length_ms / settings.step_ms - 1) : 1; // number of steps to print new line
    
    settings.no_timestamps  = !use_vad;
    settings.no_context    |= use_vad;
    settings.max_tokens     = 0;
    
    audio_input = make_unique<ofxWhisperBufferedSoundInput>(settings.length_ms);
    
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
    
    t_last  = ofGetElapsedTimeMillis();
    t_start = t_last;
    bIsSetup = true;
    startThread();
}
//-----------------------------------------------------------------------------------------------
void ofxWhisper::threadedFunction() {
    while(isThreadRunning()){
        update();
    }
}
//-----------------------------------------------------------------------------------------------
void ofxWhisper::update(){
    
    // process new audio
    if(audio_input == nullptr) return;
    
    const auto t_now  = ofGetElapsedTimeMillis();
    const auto t_diff = (t_now - t_last);
    
    if (t_diff < 2000) {
        
        // what is the point of this? collect 2000 milis of audio?
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
        sleep(100);
        
        return;
    }
    std::vector<float> pcmf32_new;
    std::vector<float> pcmf32;
    
    audio_input->get(2000, pcmf32_new);
    
    if (::vad_simple(pcmf32_new, WHISPER_SAMPLE_RATE, 1000, settings.vad_thold, settings.freq_thold, false)) {
        audio_input->get(settings.length_ms, pcmf32);
    } else {
        sleep(100);
        
        return;
    }
    
    t_last = t_now;
    
    
    // run the inference
    {
        whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
        
        wparams.print_progress   = false;
        wparams.print_special    = settings.print_special;
        wparams.print_realtime   = false;
        wparams.print_timestamps = !settings.no_timestamps;
        wparams.translate        = settings.translate;
        wparams.no_context       = true;
        wparams.single_segment   = !use_vad;
        wparams.max_tokens       = settings.max_tokens;
        wparams.language         = settings.language.c_str();
        wparams.n_threads        = settings.n_threads;
        
        wparams.audio_ctx        = settings.audio_ctx;
        wparams.speed_up         = settings.speed_up;
        
        // disable temperature fallback
        wparams.temperature_inc  = -1.0f;
        
        wparams.prompt_tokens    = settings.no_context ? nullptr : prompt_tokens.data();
        wparams.prompt_n_tokens  = settings.no_context ? 0       : prompt_tokens.size();
        
        if (whisper_full(ctx, wparams, pcmf32.data(), pcmf32.size()) != 0) {
            ofLogError("ofxWhisper::audioIn") << "failed to process audio";
            return ;
        }
        
        {
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
                if(i < n_segments -1){
                    ss << "\n";
                }
            }
            textChannel.send(ss.str());
        }
        
        ++n_iter;
    }
}
