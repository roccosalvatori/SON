# Projet SON - 3TC

In this project, I programmed an embedded system (the Teensy 4.0: https://www.pjrc.com/store/teensy40.html) for real-time audio signal processing applications. The goal is to learn the basics of audio software architecture, audio signal processing in C++, and embedded system programming (C++).

The 3 weeks project period started with a workshop on embedded real-time audio signal processing. I learned the architecture of a real-time audio DSP system (e.g., audio callback, buffering, sampling, etc.), and  various basic techniques for audio signal processing (e.g., filters, oscillators, sound synthesis techniques, sound processing techniques, sound analysis techniques, etc.).

After this period, various project ideas were suggested to focus more on DSP or on the "product/hardware" aspect.

The Arduino repository contains all the files that are nedded to operate the Teensy PCB.

The EffectsBox.ino is the main file of the project, containing the different sound effects.

The AudioSample.cpp contains an audiio file that has been converted to an array.

The wavconverter.c file is a program that converts a WAV file into an array.

