#include "ofxWhisperBufferedSoundInput.h"
#include "whisper.h"

ofxWhisperBufferedSoundInput::ofxWhisperBufferedSoundInput(int length_ms) {
    m_len_ms = length_ms;

    m_sample_rate = WHISPER_SAMPLE_RATE;
    
    m_audio.resize((m_sample_rate*m_len_ms)/1000);


}

bool ofxWhisperBufferedSoundInput::clear() {
    
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_audio_pos = 0;
        m_audio_len = 0;
    }

    return true;
}

//void ofxWhisperBufferedSoundInput::callback(uint8_t * stream, int len) {
void ofxWhisperBufferedSoundInput::audioIn( ofSoundBuffer& buffer ) {
//    if (!m_running) {
//        return;
//    }

    size_t n_samples ;

    if(WHISPER_SAMPLE_RATE != buffer.getSampleRate()){
        ofSoundBuffer converted;
        sampleRateConverter.changeSampleRate(buffer, converted, WHISPER_SAMPLE_RATE);
        
        n_samples = converted.getNumFrames();
        if(converted.getNumChannels() == 1){
            m_audio_new = converted.getBuffer();
        }else{
            m_audio_new.resize(n_samples);
            converted.copyTo(m_audio_new.data(), converted.getNumFrames(), 1, 0, false) ;
        }
    }else{
        n_samples = buffer.getNumFrames();
        if(buffer.getNumChannels() == 1){
            m_audio_new = buffer.getBuffer();
        }else{
            m_audio_new.resize(n_samples);
            buffer.copyTo(m_audio_new.data(), buffer.getNumFrames(), 1, 0, false) ;
        }
    }
 
 
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_audio_pos + n_samples > m_audio.size()) {
            const size_t n0 = m_audio.size() - m_audio_pos;

            memcpy(&m_audio[m_audio_pos], &m_audio_new[0], n0 * sizeof(float));
            memcpy(&m_audio[0], &m_audio_new[n0], (n_samples - n0) * sizeof(float));

            m_audio_pos = (m_audio_pos + n_samples) % m_audio.size();
            m_audio_len = m_audio.size();
        } else {
            memcpy(&m_audio[m_audio_pos], &m_audio_new[0], n_samples * sizeof(float));

            m_audio_pos = (m_audio_pos + n_samples) % m_audio.size();
            m_audio_len = std::min(m_audio_len + n_samples, m_audio.size());
        }
    }
}

void ofxWhisperBufferedSoundInput::get(int ms, std::vector<float> & result) {
   

    result.clear();

    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (ms <= 0) {
            ms = m_len_ms;
        }

        size_t n_samples = (m_sample_rate * ms) / 1000;
        if (n_samples > m_audio_len) {
            n_samples = m_audio_len;
        }

        result.resize(n_samples);

        int s0 = m_audio_pos - n_samples;
        if (s0 < 0) {
            s0 += m_audio.size();
        }

        if (s0 + n_samples > m_audio.size()) {
            const size_t n0 = m_audio.size() - s0;

            memcpy(result.data(), &m_audio[s0], n0 * sizeof(float));
            memcpy(&result[n0], &m_audio[0], (n_samples - n0) * sizeof(float));
        } else {
            memcpy(result.data(), &m_audio[s0], n_samples * sizeof(float));
        }
    }
}

