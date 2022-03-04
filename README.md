# opensles-alsa

Build with NDK toolchain:

make

cp .asoundrc ~/

./opensles-alsa -c 128 -C 4&

aplay frigian.wav
arecord -f S16_LE -r 44100 rec.wav

!Use alsa-lib_file_plugin.patch for alsa-lib when record audio.
