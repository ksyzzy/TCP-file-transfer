using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net.Sockets;
using System.Threading;
using System.Runtime.InteropServices; // DLL support

namespace ConsoleApp2
{
    class Program
    {

        [DllImport("SocketSupport.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int BeginServerOperation(int port);
        [DllImport("SocketSupport.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void TerminateServerConnection();

        static void Main()
        {
            int port = 0;
            bool validUserInput = false;

            while (validUserInput == false)
            {
                try
                {
                    Console.WriteLine("Please input port number:");
                    port = int.Parse(Console.ReadLine());
                }
                catch (Exception) { }

                if (port >= 1)
                {
                    validUserInput = true;
                }
                else
                {
                    Console.WriteLine("Invalid port input.");
                }

            }

            Thread thread = new Thread(new ThreadStart(Program.InputHandler));
            thread.Start();
            Console.WriteLine("Server intialized. Type in 'exit' to exit the program");
            BeginServerOperation(port);
        }

        public static void InputHandler()
        {
            while (true)
            {
                if (Console.ReadLine() == "exit")
                {
                    Console.WriteLine("Termination instruction received. Closing program...");
                    TerminateServerConnection();
                    System.Environment.Exit(1);
                }
                else
                {
                    continue;
                }
            }
        }
    }
}
