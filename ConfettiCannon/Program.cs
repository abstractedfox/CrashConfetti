/*
ConfettiCannon, copyright Chris/abstractedfox 2022. 
Launcher for CrashConfetti.exe; starts the exe when all instances of Windows Explorer exit.

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
Redistribution or modification of this code may not strike credit of the original author, Chris/abstractedfox

Contact: chriswhoprograms@gmail.com
*/


// See https://aka.ms/new-console-template for more information
using System.ComponentModel;
using System.Diagnostics;
using System.Runtime.InteropServices;



List <Process> explorerProcesses = new List<Process>();

string confettiapp = Directory.GetCurrentDirectory() + "\\ConfettiApp\\CrashConfetti.exe";
if (File.Exists(confettiapp)) Console.WriteLine("Successfully located confetti");
else
{
    Console.WriteLine(confettiapp + " contains no confetti");
    return 1;
}

GetExplorerProc();

WatchExplorer();

Console.WriteLine("i am ready to confetti");

while (true)
{
    await Task.Delay(10000);
}

bool GetExplorerProc()
{
    Console.WriteLine("Looking for explorer");
    explorerProcesses.Clear();
    Process[] processes = Process.GetProcesses();
    foreach (Process p in processes)
    {
        try
        {
            if (p != null && !p.HasExited && p.Modules != null && p.Modules.Count > 0 && p.MainModule != null)
            {
                if (p.ProcessName == "explorer")
                {
                    explorerProcesses.Add(p);
                    Console.WriteLine("Success! " + p.ToString());
                }
                if (p.ProcessName == "ConfettiCannon" && p.Id != Process.GetCurrentProcess().Id)
                {
                    //If an instance of ConfettiCannon is already running, close this instance
                    Console.WriteLine("ConfettiCannon is already running; exiting");
                    Environment.Exit(2);
                }
            }
        }
        catch (Win32Exception)
        {
            //Some processes don't like to be looked at and will throw an exception, so ignore those
        }
    }
    if (explorerProcesses.Count > 0) return true;
    return false;
}

async void WatchExplorer()
{
    await Task.Run(async () => { 
        while (true)
        {
            bool explorerAlive = true;
            await Task.Delay(500);
            for (int i = 0; i < explorerProcesses.Count; i++)
            {
                if (explorerProcesses[i].HasExited) explorerAlive = false;
                if (explorerAlive == false && !explorerProcesses[i].HasExited)
                {
                    explorerAlive = true;
                    break;
                }
            }
            if (!explorerAlive)
            {
                Console.WriteLine("CONFETTI");
                Cannon();
                while (!GetExplorerProc()) ; //Block until explorer restarts
                explorerAlive = true;
            }
        }
    });
}

void Cannon()
{
    Process.Start(confettiapp, "");
}