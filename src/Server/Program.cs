    using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices; // DLL support

namespace ConsoleApp2
{
    class Program
    {
        [DllImport("SocketSupport.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int BeginServerOperation();
        static void Main(string[] args)
        {

            BeginServerOperation();
        }
    }
}
