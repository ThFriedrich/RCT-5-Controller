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

#include <SDL.h>

// This class is a singleton, which is bad practice. However, this makes the
// implementation more straightforward.
class Beeper
{
public:
    
    static void open(); // Open the audio device
    static void close(); // Close the audio device

    static void setFrequency(double frequency); // Units: Hz
    static void setVolume(double volume); // Range: 0.0 .. 1.0

    static void play();
    static void stop();
    static void play_note(float note, uint32_t interval, bool staccato = false, float time_factor = 1.0);
    static void play_pause(uint32_t interval);
    static void super_mario_level_finshed(float speed = 1.0);
    static void super_mario_level_theme(float speed = 1.0);

    static SDL_AudioSpec m_obtainedSpec;

private:
    static SDL_AudioDeviceID m_audioDevice;
    static double m_frequency; // Units: Hz
    static double m_volume; // Range: 0.0 .. 1.0

    // The current playback position, according to `getData()` and
    // `audioCallback()`. Units: samples
    static int m_pos;

    // Pointer to function for offset calculate. Differs between different
    // audio formats.
    static int (*m_calculateOffset)(int sample, int channel);

    // Pointer to function for writing data. Differs between different audio
    // formats.
    static void (*m_writeData)(uint8_t* ptr, double data);

    // Called by `audioCallback` to generate audio data.
    static double getData();

    // This is function is called repeatedly by SDL2 to send data to the audio
    // device.
    static void audioCallback(
        void* userdata,
        uint8_t* stream,
        int len
    );

    // Note frequencies
    static constexpr float a2 = 110.00;
    static constexpr float ais2 = 116.54;
    static constexpr float b2 = 123.47;
    static constexpr float c2 = 130.81;
    static constexpr float cis2 = 138.59;
    static constexpr float d2 = 146.83;
    static constexpr float dis2 = 155.56;
    static constexpr float e2 = 164.81;
    static constexpr float f2 = 174.61;
    static constexpr float fis2 = 185.00;
    static constexpr float g2 = 196.00;
    static constexpr float gis2 = 207.65;
    static constexpr float a3 = 220.00;
    static constexpr float ais3 = 233.08;
    static constexpr float b3 = 246.94;
    static constexpr float c3 = 261.63;
    static constexpr float cis3 = 277.18;
    static constexpr float d3 = 293.66;
    static constexpr float dis3 = 311.13;
    static constexpr float e3 = 329.63;
    static constexpr float f3 = 349.23;
    static constexpr float fis3 = 369.99;
    static constexpr float g3 = 196.00;
    static constexpr float gis3 = 207.65;
    static constexpr float a4 = 220.00;
    static constexpr float ais4 = 233.08;
    static constexpr float b4 = 246.94;
    static constexpr float c4 = 261.63;
    static constexpr float cis4 = 277.18;
    static constexpr float d4 = 293.66;
    static constexpr float dis4 = 311.13;
    static constexpr float e4 = 329.63;
    static constexpr float f4 = 349.23;
    static constexpr float fis4 = 369.99;
    static constexpr float g4 = 392.00;
    static constexpr float gis4 = 415.30;
    static constexpr float a5 = 440.00;
    static constexpr float ais5 = 466.16;
    static constexpr float b5 = 493.88;
    static constexpr float c5 = 523.25;
    static constexpr float cis5 = 554.37;
    static constexpr float d5 = 587.33;
    static constexpr float dis5 = 622.25;
    static constexpr float e5 = 659.25;
    static constexpr float f5 = 698.46;
    static constexpr float fis5 = 739.99;
    static constexpr float g5 = 783.99;
    static constexpr float gis5 = 830.61;
    static constexpr float a6 = 880.00;
    static constexpr float ais6 = 932.33;
    static constexpr float b6 = 987.77;
    static constexpr float c6 = 1046.50;
    static constexpr float cis6 = 1108.73;
    static constexpr float d6 = 1174.66;
    static constexpr float dis6 = 1244.51;
    static constexpr float e6 = 1318.51;
    static constexpr float f6 = 1396.91;
    static constexpr float fis6 = 1479.98;
    static constexpr float g6 = 1567.98;
    static constexpr float gis6 = 1661.22;
    static constexpr float a7 = 1760.00;
    static constexpr float ais7 = 1864.66;
    static constexpr float b7 = 1975.53;
    static constexpr float c7 = 2093.00;

    // Note durations (in milliseconds)
    static constexpr uint32_t i1p = 2400;
    static constexpr uint32_t i1 = 1600;
    static constexpr uint32_t i2p = 1200;
    static constexpr uint32_t i2 = 800;
    static constexpr uint32_t i4p = 600;
    static constexpr uint32_t i4 = 400;
    static constexpr uint32_t i8p = 300;
    static constexpr uint32_t i8 = 200;
    static constexpr uint32_t i16p = 150;
    static constexpr uint32_t i16 = 100;
    static constexpr uint32_t i32 = 50;
};
