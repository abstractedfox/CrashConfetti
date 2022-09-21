# CrashConfetti

CrashConfetti is a fun app that produces a pop of confetti on the main monitor when Windows Explorer crashes or restarts.

How to use:
1. Run ConfettiCannon.exe
2. Wait for explorer to crash
3. Confetti!!!!!
4. ConfettiCannon.exe doesn't exit on its own after confetti, it will sit and wait for Explorer to crash again, so if you only want to see confetti once, you'll have to kill it from task manager.

The application exists in two components; a C++ program that performs the animation (CrashConfetti.exe) and exits when complete, and a C# script that watches the Explorer process and calls the confetti program when all instances exit (ConfettiCannon.exe).

The solution has both configured to build the script to /BuildBoth/Both/net6.0/, and CrashConfetti.exe to /BuildBoth/Both/net6.0/ConfettiApp. A "confetti.mp4" must also be in the ConfettiApp directory in order to work. An original confetti animation is supplied, but feel free to use your own!

If you're making your own confetti.mp4, bare in mind that the alpha channel is set to solid black (#000000), so anything other than this color will obfuscate the screen. Ideally this would be your confetti, so watch out for that. It's surprisingly difficult to get h.264 artifacts to not be ever slightly not black, but feel free to share tips or ideas if you find a good method.

It wouldn't be wise to use this in a setting where security is of high importance; verification of the confetti executable is an idea for later, but at this point it simply executes anything named "CrashConfetti.exe."

Obligatory disclaimer: By using this you software you accept sole responsibility for responsible use of this software, and you confirm that you are solely responsible for any damages that may ensue, and agree that contributors to this software shall not be held liable for any damage, loss or exfiltration of data, monetary loss, reputational damage, destruction of property, or other perceived damages to any persons, companies, or other entities.