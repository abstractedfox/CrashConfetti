# CrashConfetti, by Chris/abstractedfox, copyright 2022
# chriswhoprograms@gmail.com

CrashConfetti is a fun and simple app that produces a pop of confetti on the main monitor when Windows Explorer crashes or restarts.

How to use:
1. Run ConfettiCannon.exe
2. Wait for explorer to crash
3. Confetti!!!!!

To start at login: *Broken, see known issues
Simply place the Startup.ps1 Powershell script in the same folder as the ConfettiCannon.exe file and run it with Powershell. A shortcut to the launcher will be placed in your startup folder. To remove it, delete it from the startup folder (Run > 'shell:startup' and delete 'ConfettiCannon.lnk')

To start at login while the powershell script doesn't work:
Create a shortcut to ConfettiCannon.exe. Open the "Run" prompt and type "shell:startup" and place the shortcut there.

ConfettiCannon.exe doesn't exit after confetti; it will sit and wait for Explorer to crash again, so if you only want to see confetti once, you'll have to kill it from task manager.

Useful info:
Finding the explorer processes takes a moment (approx ~10 seconds on my machine), so if you're restarting Explorer manually to test it out, make sure to give it a moment first.

It wouldn't be wise to use this in a setting where security is of high importance; verification of the confetti executable is an idea for later, but at this point it simply executes anything named "CrashConfetti.exe."

Some further enhancements will likely come in the future, so to further improve your confetti experience, it's encouraged to check in now and then.

If you have fun using this, it would be fun to hear about it! 


Known issues:
Startup.ps1 produces a broken shortcut
For reasons unknown, the script seems to produce a working shortcut as long as it's placed anywhere other than the startup folder. If you figure out what's wrong here, I'd love to know.

Window border is visible
Winapi seems to offer variable opacity for an entire window, or a single alpha channel that is locked to fully transparent (which is what is currently in use.) I'm not convinced there isn't a possible workaround here, so future releases may look a little better

Information about the program code:
The application exists in two components; a C++ program that plays the animation (CrashConfetti.exe) and exits when complete, and a C# script that watches the Explorer process and calls the confetti program when all instances exit (ConfettiCannon.exe).

The solution configuration "BothProjects" has both configured to build the script to /BuildBoth/Both/net6.0/, and CrashConfetti.exe to /BuildBoth/Both/net6.0/ConfettiApp. A "confetti.mp4" must also be in the ConfettiApp directory, or it won't have any confetti to explode!

If you're making your own confetti.mp4, bare in mind that the alpha channel is set to solid black (#000000), so anything other than this color will obfuscate the screen. Ideally this would be your confetti, so watch out for that. It's surprisingly difficult to get h.264 artifacts to not be ever slightly not black, but feel free to share tips or ideas if you find a good method.


Licensing info:
This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.

In addition to the standard GPL v3.0, this program includes an additional condition:
Redistribution or modification of this code may not strike accredidation of the original author, Chris/abstractedfox
