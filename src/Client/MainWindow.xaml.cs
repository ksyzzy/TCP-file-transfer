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
    public partial class MainWindow : Window
    {
        static int port;
        static char[] host;

        [DllImport("SocketSupport.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int BeginClientOperation(char[] host, int port);
        [DllImport("SocketSupport.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetFileList();
        [DllImport("SocketSupport.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void TerminateClientConnection();
        [DllImport("SocketSupport.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int CheckConnectionStatus();
        public MainWindow()
        {
            InitializeComponent();

        }

        public static void Connector()
        {
            MessageBox.Show("Return code: " + Convert.ToString(BeginClientOperation(host, port)));
            return;
        }

        private void TryConnecting_Click(object sender, RoutedEventArgs e)
        {
            string hostname;
            hostname = hostnameInput.Text;
            host = hostname.ToCharArray();
            port = Convert.ToInt32(portInput.Text);

            Thread thread = new Thread(
            new ThreadStart(MainWindow.Connector));
            thread.Start();
            MessageBox.Show("Connected successfully");

        }

        private void portInput_TextChanged(object sender, TextChangedEventArgs e)
        {

        }

        private void TextBox_TextChanged(object sender, TextChangedEventArgs e)
        {

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

        private void GetList_Click(object sender, RoutedEventArgs e)
        {
            if (CheckConnectionStatus() == 1)
            {
                switch (GetFileList())
                {
                    case 1:
                        MessageBox.Show("File downloaded successfully");
                        break;
                    case -1:
                        MessageBox.Show("Error: File is empty");
                        break;
                    case -2:
                        MessageBox.Show("Error: Failed to receive filelist size");
                        break;
                    case -9:
                        MessageBox.Show("Error: Unknown error has occured");
                        break;
                }
            }
            else
            {
                MessageBox.Show("You have to establish a connection to the server first");
            }
        }

        private void TerminateConnection_Click(object sender, RoutedEventArgs e)
        {
            TerminateClientConnection();
            return;
        }
    }
}

