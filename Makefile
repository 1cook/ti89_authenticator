CC = tigcc
CFLAGS = -std=c99 -Wall -W -Os -ffunction-sections -fdata-sections -fomit-frame-pointer -Wa,--all-relocs,-l -mregparm=5 -mno-bss -DMIN_AMS=207 -DUSE_FLINE_ROM_CALLS -save-temps

all: bin/totp.89z

bin/totp.89z: bin/sha1.o bin/hmac.o bin/hotp.o bin/time.o bin/tile.o bin/secret_file.o bin/secret_util.o bin/secret_manifest.o bin/new_secret.o bin/time_zone.o bin/main_menu.o totp.c
	chmod 700 commit_hash.sh
	./commit_hash.sh
	$(CC) $(CFLAGS) -v --optimize-code --cut-ranges --reorder-sections --remove-unused --merge-constants bin/sha1.o bin/hmac.o bin/hotp.o bin/time.o bin/tile.o bin/secret_file.o bin/secret_util.o bin/secret_manifest.o bin/new_secret.o bin/time_zone.o bin/main_menu.o totp.c -o bin/totp.89z

bin/time.o: time/time.c
	$(CC) $(CFLAGS) -c time/time.c -o bin/time.o
bin/hotp.o: codes/hotp/hotp.c
	$(CC) $(CFLAGS) -c codes/hotp/hotp.c -o bin/hotp.o
bin/sha1.o: codes/hash/sha1/sha1/sha1.c
	$(CC) $(CFLAGS) -c codes/hash/sha1/sha1/sha1.c -o bin/sha1.o
bin/hmac.o: codes/hmac/hmac.c
	$(CC) $(CFLAGS) -c codes/hmac/hmac.c -o bin/hmac.o
bin/tile.o: gfx/tile.c
	$(CC) $(CFLAGS) -c gfx/tile.c -o bin/tile.o
bin/secret_file.o: save/secret_file.c
	$(CC) $(CFLAGS) -c save/secret_file.c -o bin/secret_file.o
bin/secret_util.o: save/util.c
	$(CC) $(CFLAGS) -c save/util.c -o bin/secret_util.o
bin/secret_manifest.o: save/manifest.c
	$(CC) $(CFLAGS) -c save/manifest.c -o bin/secret_manifest.o
bin/new_secret.o: dialog/new_secret.c
	$(CC) $(CFLAGS) -c dialog/new_secret.c -o bin/new_secret.o
bin/time_zone.o: dialog/time_zone.c
	$(CC) $(CFLAGS) -c dialog/time_zone.c -o bin/time_zone.o
bin/main_menu.o: mainmenu/main_menu.c
	$(CC) $(CFLAGS) -c mainmenu/main_menu.c -o bin/main_menu.o

clean:
	rm -rf bin
	mkdir bin
