# CrashConfetti

CrashConfetti is a fun app that produces a pop of confetti on the main monitor when Windows Explorer crashes or restarts.

The application exists in two components; a C++ program that performs the animation and exits when complete, and a C# script that watches the Explorer process and calls the confetti program when all instances exit.

The solution has both configured to build into the "BuildBoth" directory. All output files, and an original "confetti.mp4", must all be in the same directory in order to work. Until there is an original confetti animation, you will have to supply your own.

If you're making your own confetti.mp4, bare in mind that the alpha channel is set to solid black, so anything other than this color will obfuscate the screen. Ideally this would be your confetti, so watch out for that.

It wouldn't be wise to use this in a setting where security is of high importance; verification of the confetti executable may be added later, but at this point it simply executes anything named "CrashConfetti.exe" in its working directory.

Obligatory disclaimer: By using this you software you accept sole responsibility for responsible use of this software, and you confirm that you are solely responsible for any damages that may ensue, and agree that contributors to this software shall not be held liable for any damage, loss or exfiltration of data, monetary loss, reputational damage, destruction of property, or other perceived damages to any persons, companies, or other entities.