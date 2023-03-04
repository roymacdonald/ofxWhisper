#pragma once

// needs to match WHISPER_SAMPLE_RATE
#define COMMON_SAMPLE_RATE 16000

#include <vector>
#include <string>

std::string trim(const std::string & s);

std::string replace(
        const std::string & s,
        const std::string & from,
        const std::string & to);

// Apply a high-pass frequency filter to PCM audio
// Suppresses frequencies below cutoff Hz
void high_pass_filter(
        std::vector<float> & data,
        float cutoff,
        float sample_rate);

// Basic voice activity detection (VAD) using audio energy adaptive threshold
bool vad_simple(
        std::vector<float> & pcmf32,
        int   sample_rate,
        int   last_ms,
        float vad_thold,
        float freq_thold,
        bool  verbose);

