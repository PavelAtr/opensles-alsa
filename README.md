# opensles-alsa

Build with NDK toolchain:

make

cp .asoundrc ~/

./opensles-alsa -c 128&

aplay frigian.wav

!Use alsa-lib_file_plugin.patch for alsa-lib when record audio.
