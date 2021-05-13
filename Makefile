CC = tigcc
CFLAGS = -std=c99 -Wall -W -Os -ffunction-sections -fdata-sections -fomit-frame-pointer -Wa,--all-relocs,-l -mregparm=5 -mno-bss -DMIN_AMS=207 -DUSE_FLINE_ROM_CALLS -save-temps

all: bin/totp.89z

bin/totp.89z: codes/hash/sha1/sha1/sha1.c codes/hmac/hmac.c codes/hotp/hotp.c time/time.c gfx/tile.c save/secret_file.c save/util.c save/manifest.c dialog/new_secret.c dialog/time_zone.c mainmenu/main_menu.c totp.c
	chmod 700 commit_hash.sh
	./commit_hash.sh
	$(CC) $(CFLAGS) -v --optimize-code --cut-ranges --reorder-sections --remove-unused --merge-constants totp.c -o bin/totp.89z

clean:
	rm -rf bin
	mkdir bin
