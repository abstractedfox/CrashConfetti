// See https://aka.ms/new-console-template for more information
using System.ComponentModel;
using System.Diagnostics;
using System.Runtime.InteropServices;



List < Process > explorerprocesses = new List<Process>();

string confettiapp = Directory.GetCurrentDirectory() + "\\CrashConfetti.exe";
if (File.Exists(confettiapp)) Console.WriteLine("Successfully located confetti");
else
{
    Console.WriteLine(confettiapp + " contains no confetti");
    return -1;
}


getExplorerProc();

watchExplorer();

Console.WriteLine("i am ready to confetti");

while (true)
{
    await Task.Delay(10000);
}

bool getExplorerProc()
{
    Console.WriteLine("Looking for explorer");
    explorerprocesses.Clear();
    Process[] processes = Process.GetProcesses();
    foreach (Process p in processes)
    {
        try
        {
            if (p != null && !p.HasExited && p.Modules != null && p.Modules.Count > 0 && p.MainModule != null)
            {
                if (p.ProcessName == "explorer")
                {
                    explorerprocesses.Add(p);
                    Console.WriteLine("Success! " + p.ToString());
                }
            }
        }
        catch (Win32Exception)
        {

        }
    }
    if (explorerprocesses.Count > 0) return true;
    return false;
}

async void watchExplorer()
{
    await Task.Run(async () => { 
        while (true)
        {
            bool explorerAlive = true;
            await Task.Delay(500);
            for (int i = 0; i < explorerprocesses.Count; i++)
            {
                if (explorerprocesses[i].HasExited) explorerAlive = false;
                if (explorerAlive == false && !explorerprocesses[i].HasExited)
                {
                    explorerAlive = true;
                    break;
                }
            }
            if (!explorerAlive)
            {
                Console.WriteLine("CONFETTI");
                cannon();
                while (!getExplorerProc()) ; //Block until explorer restarts
                explorerAlive = true;
            }
            //else Console.WriteLine("Explorer moment");
        }
    });
}

void cannon()
{
    Process.Start(confettiapp, "");
}