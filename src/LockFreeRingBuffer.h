
//
//  LockFreeRingBuffer.hpp
//  BufferedAudioInput
//
//  Created by Roy Macdonald on 02-09-23.
//
// This is a heavily modified version of the RingBuffer class from ofxHapPlayer. 
// Its copyright goes below
/*
 RingBuffer.h
 ofxHapPlayer

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

#pragma once
#include <atomic>
#include <vector>
#include "ofSoundBuffer.h"



///\brief A lock-free ring buffer for float (audio) samples
class LockFreeRingBuffer {
public:
    ///\brief constructor
    ///\param size is the number of samples  the buffer to allocate
    LockFreeRingBuffer(size_t size);
    
    ///\brief gets the number of samples
    size_t size() const {return _buffer.size();}
    
    ///\brief sets the number of samples . This will change the size of the buffer.
    ///\param size is the number of samples  the buffer to allocate
    void allocate(size_t size);
    
    ///\brief Read this buffer's data and put into the passed ofSoundBuffer
    ///\param buffer reference to the buffer where the data is going to be placed.
    /// It is important that the passed buffer is properly allocated and setup, otherwise errors will arise
    void readIntoBuffer(ofSoundBuffer& buffer);
    
    ///\brief Read this buffer's data and put into the passed vector
    ///\param data reference to the vector where the data is going to be placed.
    /// It is important that the passed vector is properly allocated to the amount of data that needs to be read
    void readIntoVector(std::vector<float>& data);
    
    ///\brief Write  the passed ofSoundBuffer's data into the ringBuffer
    ///\param buffer reference to the buffer where the data is going to be taken from.
    /// It is important that the passed buffer is properly allocated and setup, otherwise errors will arise
    void writeFromBuffer(const ofSoundBuffer& buffer);
    
    
    
    size_t getReadPosition();
    
    ///\brief get the number of readable samples
    size_t getNumReadableSamples() const;

private:
    
    // On return first and second are pointers to positions to write samples to
    // firstCount and secondCount are the number of samples that can be written
    // returns the number of writeable samples.
    size_t writeBegin(float * &first, size_t &firstCount, float * &second, size_t &secondCount);
    
    ///\brief Call this function after finished writting. It will move the writeIndex by numSamples
    ///\param numSamples is the number of samples  actually that were written.
    void writeEnd(size_t numSamples);
    
    // On return first and second are pointers to positions to read samples from
    // firstCount and secondCount are the number of samples that can be read
    // returns the number of readable samples.
    size_t readBegin(const float * &first, size_t &firstCount, const float * &second, size_t &secondCount );
    
    ///\brief Call this function after finished reading. will move the readIndex by numSamples
    ///\param numSamples is the number of samples that where actually read
    void readEnd(size_t numSamples);

    
    
    std::atomic<size_t>    _readStart;
    std::atomic<size_t>    _writeStart;
    std::vector<float>     _buffer;
    size_t                 _samples;

};



