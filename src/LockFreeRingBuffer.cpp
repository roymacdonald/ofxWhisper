//
//  LockFreeRingBuffer.cpp
//  BufferedAudioInput
//
//  Created by Roy Macdonald on 07-09-23.
//


// This is a heavily modified version of the RingBuffer class from ofxHapPlayer. 
// Its copyright goes below
/*
 RingBuffer.cpp

 Copyright (c) 2016, Tom Butterworth. All rights reserved.
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "LockFreeRingBuffer.h"

#include <algorithm>
#include <iostream>
#include "ofLog.h"



LockFreeRingBuffer::LockFreeRingBuffer(size_t size)
:
_readStart(0),
_writeStart(0)
{

allocate(size);

}

//------------------------------------------------------------------------------------------------
void LockFreeRingBuffer::allocate(size_t size){
    _samples = size;
    _buffer.resize(size ); 
}

//------------------------------------------------------------------------------------------------
size_t LockFreeRingBuffer::getNumReadableSamples() const{
    auto writeStart = _writeStart.load();
    auto readStart = _readStart.load();

    
    size_t readable = 0;
    if(writeStart > readStart){
        return std::min(writeStart - readStart, _samples);
    }
    return 0;
}
//------------------------------------------------------------------------------------------------
size_t LockFreeRingBuffer::writeBegin(float *&first, size_t &firstCount, float *&second, size_t &secondCount)
{
    auto writeStart = _writeStart.load();
    auto readStart = _readStart.load();

    
    size_t readable = 0;
    if(writeStart > readStart){
        readable = std::min(writeStart - readStart, _samples);
    }
    
    size_t writable = _samples - readable;
    
    auto readPosition = readStart % _samples ;
    auto writePosition = writeStart % _samples;
    
    first = &_buffer[writePosition];
    second = &_buffer[0];

    if(writePosition >= readPosition){
        firstCount = _samples - writePosition;
        secondCount = readPosition;
    }else{
        firstCount = writable;
        secondCount = 0;
    }    
    return writable;
}
//------------------------------------------------------------------------------------------------
void LockFreeRingBuffer::writeEnd(size_t numSamples)
{
    _writeStart += numSamples;
}
//------------------------------------------------------------------------------------------------
size_t LockFreeRingBuffer::readBegin(const float *&first, size_t &firstCount, const float *&second, size_t &secondCount)
{
    auto writeStart = _writeStart.load();
    auto readStart = _readStart.load();

    
    size_t readable = 0;
    if(writeStart > readStart){
        readable = std::min(writeStart - readStart, _samples);
    }
    
    size_t writeable = _samples - readable;
    
    auto readPosition = readStart % _samples ;
    auto writePosition = writeStart % _samples;

    first = &_buffer[readPosition];
    second = &_buffer[0];
    
    if(writePosition >= readPosition){
        firstCount = readable;
        secondCount = 0;
    }else{
        firstCount = _samples  - readPosition;
        secondCount = writePosition;
    }
    
    return readable;
}

//------------------------------------------------------------------------------------------------
void LockFreeRingBuffer::readEnd(size_t numSamples)
{
    _readStart += numSamples;
}

//------------------------------------------------------------------------------------------------
size_t LockFreeRingBuffer::getReadPosition()
{
    return (_readStart.load() % _samples) ;
}
//------------------------------------------------------------------------------------------------
void LockFreeRingBuffer::readIntoVector(std::vector<float>& data){
    size_t wanted = data.size();
    size_t filled = 0;
    
    const float *src[2];
    size_t count[2];
    
    
    readBegin(src[0], count[0], src[1], count[1]);
    
    for (size_t i = 0; i < 2; i++)
    {
        size_t todo = std::min(wanted - filled, count[i]);
        if (todo > 0)
        {
            float *out = &data[filled];
            memcpy(out, src[i], todo * sizeof(float) );
            filled += todo;
        }
    }
    
    readEnd(filled);

    if (filled < wanted)
    {
        ofLogWarning("LockFreeRingBuffer::readIntoVector") << "filled < wanted. " << filled << " < " << wanted;
        float *out = &data[filled];
        memset(out, 0, (wanted - filled) * sizeof(float) );
    }
}
//------------------------------------------------------------------------------------------------
void LockFreeRingBuffer::readIntoBuffer(ofSoundBuffer& buffer)
{
    readIntoVector(buffer.getBuffer());
//    size_t wanted = buffer.size();
//    size_t filled = 0;
//
//    const float *src[2];
//    size_t count[2];
//
//
//    readBegin(src[0], count[0], src[1], count[1]);
//
//    for (size_t i = 0; i < 2; i++)
//    {
//        size_t todo = std::min(wanted - filled, count[i]);
//        if (todo > 0)
//        {
//            float *out = &buffer.getBuffer()[filled];
//            memcpy(out, src[i], todo * sizeof(float) );
//            filled += todo;
//        }
//    }
//
//    readEnd(filled);
//
//    if (filled < wanted)
//    {
//        ofLogWarning("LockFreeRingBuffer::readIntoBuffer") << "filled < wanted. " << filled << " < " << wanted;
//        float *out = &buffer.getBuffer()[filled];
//        memset(out, 0, (wanted - filled) * sizeof(float) );
//    }
    
}

//------------------------------------------------------------------------------------------------
void LockFreeRingBuffer::writeFromBuffer(const ofSoundBuffer& buffer){

    size_t wanted = buffer.size();
    float *dst[2];
    size_t count[2];
    size_t consumed = 0;
    
    writeBegin(dst[0], count[0], dst[1], count[1]);
    
    
    for(size_t i = 0; i < 2; i++){
        
        size_t todo = std::min(wanted - consumed, count[i]);
        if(todo > 0){
            const float *in = &buffer.getBuffer()[consumed];
            memcpy(dst[i], in,  todo *  sizeof(float) );
            consumed += todo;
        }
    }
    writeEnd(consumed);
    if(consumed < wanted){
        ofLogWarning("LockFreeRingBuffer::writeFromBuffer") << "consumed < wanted. " << consumed << " < " << wanted;
    }
}
