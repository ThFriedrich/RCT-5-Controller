// SDL2 Beeper
// Copyright © 2022 qxxxb

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <iostream>
#include <string>
#include <math.h>

#include "beeper.h"
#include "Utilities.h"

SDL_AudioDeviceID Beeper::m_audioDevice;
SDL_AudioSpec Beeper::m_obtainedSpec;
double Beeper::m_frequency;
double Beeper::m_volume;
int Beeper::m_pos;
void (*Beeper::m_writeData)(uint8_t *ptr, double data);
int (*Beeper::m_calculateOffset)(int sample, int channel);

// ---
// Calculate the offset in bytes from the start of the audio stream to the
// memory address at `sample` and `channel`.
//
// Channels are interleaved.

int calculateOffset_s16(int sample, int channel)
{
    return (sample * sizeof(int16_t) * Beeper::m_obtainedSpec.channels) +
           (channel * sizeof(int16_t));
}

int calculateOffset_f32(int sample, int channel)
{
    return (sample * sizeof(float) * Beeper::m_obtainedSpec.channels) +
           (channel * sizeof(float));
}

// ---
// Convert a normalized data value (range: 0.0 .. 1.0) to a data value matching
// the audio format.

void writeData_s16(uint8_t *ptr, double data)
{
    int16_t *ptrTyped = (int16_t *)ptr;
    double range = (double)INT16_MAX - (double)INT16_MIN;
    double dataScaled = data * range / 2.0;
    *ptrTyped = dataScaled;
}

void writeData_f32(uint8_t *ptr, double data)
{
    float *ptrTyped = (float *)ptr;
    *ptrTyped = data;
}

// ---
// Generate audio data. This is how the waveform is generated.

double Beeper::getData()
{
    double sampleRate = (double)(m_obtainedSpec.freq);

    // Units: samples
    double period = sampleRate / m_frequency;

    // Reset m_pos when it reaches the start of a period so it doesn't run off
    // to infinity (though this won't happen unless you are playing sound for a
    // very long time)
    if (m_pos % (int)period == 0)
    {
        m_pos = 0;
    }

    double pos = m_pos;
    double angular_freq = (1.0 / period) * 2.0 * M_PI;
    double amplitude = m_volume;

    return sin(pos * angular_freq) * amplitude;
}

void Beeper::audioCallback(
    void *userdata,
    uint8_t *stream,
    int len)
{
    // Unused parameters
    (void)userdata;
    (void)len;

    // Write data to the entire buffer by iterating through all samples and
    // channels.
    for (int sample = 0; sample < m_obtainedSpec.samples; ++sample)
    {
        double data = getData();
        m_pos++;

        // Write the same data to all channels
        for (int channel = 0; channel < m_obtainedSpec.channels; ++channel)
        {
            int offset = m_calculateOffset(sample, channel);
            uint8_t *ptrData = stream + offset;
            m_writeData(ptrData, data);
        }
    }
}

void Beeper::open()
{
    // First define the specifications we want for the audio device
    SDL_AudioSpec desiredSpec;
    SDL_zero(desiredSpec);

    // Commonly used sampling frequency
    desiredSpec.freq = 44100;

    // Currently this program supports two audio formats:
    // - AUDIO_S16: 16 bits per sample
    // - AUDIO_F32: 32 bits per sample
    //
    // We need this flexibility because some sound cards do not support some
    // formats.

    // Higher bit depth means higher resolution the sound, lower bit depth
    // means lower resolution for the sound. Since we are just playing a simple
    // sine wave, 16 bits is fine.
    desiredSpec.format = AUDIO_S16;

    // Smaller buffer means less latency with the sound card, but higher CPU
    // usage. Bigger buffers means more latency with the sound card, but lower
    // CPU usage. 512 is fairly small, since I don't want a delay before a beep
    // is played.
    desiredSpec.samples = 512;

    // Since we are just playing a simple sine wave, we only need one channel.
    desiredSpec.channels = 1;

    // Set the callback (pointer to a function) to our callback. This function
    // will be called by SDL2 in a separate thread when it needs to write data
    // to the audio buffer. In other words, we don't control when this function
    // is called; SDL2 manages it.
    desiredSpec.callback = Beeper::audioCallback;

    // When we open the audio device, we tell SDL2 what audio specifications we
    // desire. SDL2 will try to get these specifications when opening the audio
    // device, but sometimes the audio device does not support some of our
    // desired specifications. In that case, we have to be flexible and adapt
    // to what the audio device supports. The obtained specifications that the
    // audio device supports will be stored in `m_obtainedSpec`

    m_audioDevice = SDL_OpenAudioDevice(
        NULL,            // device (name of the device, which we don't care about)
        0,               // iscapture (we are not recording sound)
        &desiredSpec,    // desired
        &m_obtainedSpec, // obtained
        0                // allowed_changes (allow any changes between desired and obtained)
    );

    if (m_audioDevice == 0)
    {
        SDL_Log("Failed to open audio: %s", SDL_GetError());
        // TODO: throw exception
    }
    else
    {
        std::string formatName;
        switch (m_obtainedSpec.format)
        {
        case AUDIO_S16:
            m_writeData = writeData_s16;
            m_calculateOffset = calculateOffset_s16;
            formatName = "AUDIO_S16";
            break;
        case AUDIO_F32:
            m_writeData = writeData_f32;
            m_calculateOffset = calculateOffset_f32;
            formatName = "AUDIO_F32";
            break;
        default:
            SDL_Log("Unsupported audio format: %i", m_obtainedSpec.format);
            // TODO: throw exception
        }
    }
}

void Beeper::close()
{
    SDL_CloseAudioDevice(m_audioDevice);
}

void Beeper::setFrequency(double frequency)
{
    m_frequency = frequency;
}

void Beeper::setVolume(double volume)
{
    m_volume = volume;
}

void Beeper::play()
{
    SDL_PauseAudioDevice(m_audioDevice, 0);
}

void Beeper::stop()
{
    SDL_PauseAudioDevice(m_audioDevice, 1);
}

void Beeper::play_note(float note, uint32_t interval, bool staccato, float time_factor)
{
    interval *= time_factor;
    size_t duration_note = static_cast<size_t>(interval);
    size_t duration_silence = 0;
    setFrequency(note);
    if (staccato)
    {
        duration_silence = static_cast<size_t>(interval * 0.1);
        duration_note = static_cast<size_t>(interval * 0.9);
    }
    sleep(duration_note);
    if (staccato)
    {
        setVolume(0.0);
        sleep(duration_silence);
        setVolume(1.0);
    }
}

void Beeper::super_mario_level_finshed(float speed)
// Play super mario bros winner theme
{

    open();
    setVolume(1.0);
    play();

    play_note(g3, i8, false, speed);
    play_note(c4, i8, false, speed);
    play_note(e4, i8, false, speed);
    play_note(g4, i8, false, speed);
    play_note(c5, i8, false, speed);
    play_note(e5, i8, false, speed);
    play_note(g5, i4p, false, speed);
    play_note(e5, i4p, false, speed);

    play_note(gis3, i8, false, speed);
    play_note(c4, i8, false, speed);
    play_note(dis4, i8, false, speed);
    play_note(gis4, i8, false, speed);
    play_note(c5, i8, false, speed);
    play_note(dis5, i8, false, speed);
    play_note(gis5, i4p, false, speed);
    play_note(dis5, i4p, false, speed);

    play_note(ais4, i8, false, speed);
    play_note(d4, i8, false, speed);
    play_note(f4, i8, false, speed);
    play_note(ais5, i8, false, speed);
    play_note(d5, i8, false, speed);
    play_note(f5, i8, false, speed);
    play_note(ais6, i4p, true, speed);
    play_note(ais6, i8, true, speed);
    play_note(ais6, i8, true, speed);
    play_note(ais6, i8, true, speed);
    play_note(c6, i2p, false, speed);

    stop();
    close();
}
