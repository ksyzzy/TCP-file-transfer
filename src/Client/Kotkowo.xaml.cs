using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Runtime.InteropServices; // DLL support
using System.Threading;


namespace Client
{
    /// <summary>
    /// Logika interakcji dla klasy MainWindow.xaml
    /// </summary>
    public partial class Kotkowo : Window
    {
        static string hostname;
        static int port;
        static int hostLength;
        static char[] host;

        [DllImport("SocketSupport.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int BeginClientOperation(char[] host, int port, int hostLen);
        [DllImport("SocketSupport.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetFileList();
        [DllImport("SocketSupport.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void TerminateClientConnection();
        [DllImport("SocketSupport.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int CheckConnectionStatus();
        [DllImport("SocketSupport.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int CheckClient();
        [DllImport("SocketSupport.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int DownloadImage(char[] imageName);
        [DllImport("SocketSupport.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int UploadImage(char[] imageName);

        public Kotkowo()
        {
            InitializeComponent();
            this.Title = "Kotkowo";
        }

        public static void Connector()
        {
            MessageBox.Show("Connection closed. Return code: " + Convert.ToString(BeginClientOperation(host, port, hostLength)));
            return;
        }

        public static void Detector(object obj)
        {

            Thread thread = (Thread)obj;

            while (true)
            {
                if (!thread.IsAlive)
                {
                    return;
                }
                else
                {
                    if (CheckConnectionStatus() == 1)
                    {
                        return;
                    }
                    else
                    {
                        System.Threading.Thread.Sleep(100);
                        continue;
                    }
                }
            }
        }

        private void TryConnecting_Click(object sender, RoutedEventArgs e)
        {
            if (CheckClient() == 1)
            {
                MessageBox.Show("Connection has already been established");
                return;
            }

            hostname = hostnameInput.Text;
            host = hostname.ToCharArray();
            hostLength = hostname.Length;

            try
            {
                port = Convert.ToInt32(portInput.Text);
            }
            catch (Exception) { }

            if (port < 1)
            {
                MessageBox.Show("Invalid port number");
                return;
            }

            Thread thread = new Thread(
            new ThreadStart(Kotkowo.Connector));
            thread.Start();

            Thread thread2 = new Thread(new ParameterizedThreadStart(Detector));
            thread2.Start(thread);

            thread2.Join();

            if (CheckConnectionStatus() == 1)
            {
                MessageBox.Show("Connected successfully");
                Window1 connected = new Window1();
                connected.Show();
                GC.Collect();
                GC.WaitForPendingFinalizers();
                GC.Collect();
                this.Close();
            }
            else
            {
                MessageBox.Show("Unable to connect");
            }

        }

        private void CheckStatus_Click(object sender, RoutedEventArgs e)
        {
            if (CheckConnectionStatus() == 1)
            {
                MessageBox.Show("Connected");
            }
            else
            {
                MessageBox.Show("No connection established");
            }
        }

        private void TextBox_TextChanged(object sender, TextChangedEventArgs e)
        {

        }

        private void portInput_TextChanged(object sender, TextChangedEventArgs e)
        {

        }
    }
}
