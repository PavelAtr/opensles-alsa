INCLUDES = \
-I /usr/include/android/nativehelper/ \
-I ./include


opensles-alsa:
	$(CC) $(CFLAGS) $(INCLUDES) opensl_io.c fifo.c -lOpenSLES $(LDFLAGS) -o opensles-alsa

install:
	install -D -m 755 opensles-alsa $(DESTDIR)/usr/bin/opensles-alsa || true
	install -D -m 644 .asoundrc $(DESTDIR)/etc/skel/.asoundrc || true
	install -D -m 644 .jackdrc $(DESTDIR)/etc/skel/.jackdrc || true

clean:
	rm opensles-alsa || true


