# FM synthesizer
Synthesizer using frequency modulation synthesis with 8 operators


A virtual synthesizer using frequency modulation synthesis. It has 8 operators and for each operator it is possible to choose between the waveforms sine wave, sawtooth wave, square wave and triangle wave. For each operator there is an ADSR amplitude envelope filter. It is possible to connect a MIDI keyboard. There is a sequencer and it is possible to export to a wave file.

To complile: cl gen_h.c soundFactory.c -link user32.lib Gdi32.lib

The sound processing takes place in the soundfactory.c file, mainly in the soundFactoryProduce() function.

A short demonstration: https://youtu.be/k6hJZuY6YwA
