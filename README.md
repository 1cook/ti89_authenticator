# ti89_authenticator
TOTP Authenticator App for Texas instruments calcualtors

Authenticator app (RFC 6238) for the Ti-89 and Ti-92 series of calculators. This app is compatible with google authenticator.

## Install 
Running `make` within the main directory produces file file bin/totp.89z. You must have `make` and `gcc4ti` installed. Compilation works on linux only.
I will include a pre-compiled binary file in the [releases](https://github.com/1cook/ti89_authenticator/releases).

This single file will run on The Ti-89*, Ti-92 Plus*, Ti-89 Titanium, and Voyage 200.

Once you have the binary, transfer the file to a calculator using a linking program such as [TiLP](http://lpg.ticalc.org/prj_tilp/) or Ti-Graphlink.

(*) It should theoretically work on these calcualtors, but I have not tested it on them because I do not own these models.

## Time settings
You must ensure the system clock is set. Incorrect time settings will cause incorrect codes.

You will be asked to provide your own time-zone as a UTC offset. You can find yours [here](https://en.wikipedia.org/wiki/List_of_UTC_time_offsets). If you are currently observing daylight savings time, add 1 hour to your offset. When the clock switches from daylight savings to standard time, change the offset back to its original value.

If the app thinks that the clock is not set, it will ask you to set it.

## Controls
From the main menu:

+ Arrows - Select a secret. If there are more than a screenful of secrets, the screen can be scrolled down.
+ F1 - Create a new secret
+ F2 - Open a secret already saved as a variable
+ F3 - Remove the selected from display. The secret will remain saved in the file-system.
+ F4 - Reset timezone.
+ F5 - Display about message.
+ Enter - Display the filename of the selected secret.
+ ESC - Close the app.

## File structure
All files are stored in the `secrets` directory.

A special file named `manifest` stores the list of all displayed secret files, as well as settings. It must neither be locked nor archived.

Secret files have extension `secr` and contain your secret keys. They can be archived or stored in RAM. Files are stored in RAM by default.

## Caution
Use paper backup codes if your service offers them. Archive your secrets to prevent loss on battery depletion.

Do not share your secret keys or codes.

## Why are my codes wrong? What to do about it?
List of possible reasons, in decreasing order of likelihood:
+ The clock was reset (because the battery was disconnected). Set the clock by closing the app, pressing F1 and selecting "Clock".
+ The time-zone is incorrect. Set the time-zone by pressing F4 from the main menu.
+ The date/time is incorrect. Set the date/time.
+ You entered the secret key incorrectly into the calculator. Delete the file by selecting it and pressing F3. Then press F1 to try adding the file again.
+ There is a bug. Raise an [issue](https://github.com/1cook/ti89_authenticator/issues) in github. Be detailed; provide exact instructions on how to reproduce the bug, include the particular secret that you used (after you replaced the secret with your service).
