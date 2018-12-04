using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Drawing;
using System.Windows.Input;
using System.Windows.Forms;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using System.Runtime.InteropServices; // DLL support
using System.IO;

namespace Client
{
    /// <summary>
    /// Logika interakcji dla klasy Window1.xaml
    /// </summary>
    public partial class Window1 : Window
    {
        [DllImport("SocketSupport.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void TerminateClientConnection();
        [DllImport("SocketSupport.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int CheckConnectionStatus();
        [DllImport("SocketSupport.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetFileList();
        [DllImport("SocketSupport.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int DownloadImage([MarshalAs(UnmanagedType.LPArray)] char[] imageName, long length);
        [DllImport("SocketSupport.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int UploadImage([MarshalAs(UnmanagedType.LPArray)] char[] imageName, long length);

        public Window1()
        {
            string[] lines;
            InitializeComponent();
            this.Title = "Kotkowo";

            if (CheckConnectionStatus() == 1)
            {
                switch (GetFileList())
                {
                    case 1:
                        System.Windows.MessageBox.Show("Filelist received successfully. Analyzing...");
                        lines = System.IO.File.ReadAllLines("catlist.txt");

                        foreach (string line in lines)
                        {
                            lstBox.Items.Add(line);
                        }
                        break;
                    case -1:
                        System.Windows.MessageBox.Show("Error: Filelist on the server is empty. Aborting...");
                        TerminateClientConnection();
                        System.Environment.Exit(1);
                        break;
                    case -2:
                        System.Windows.MessageBox.Show("Error: Failed to receive filelist size. Aborting...");
                        TerminateClientConnection();
                        System.Environment.Exit(1);
                        break;
                    case -9:
                        System.Windows.MessageBox.Show("Error: Unknown error has occured. Aborting...");
                        TerminateClientConnection();
                        System.Environment.Exit(1);
                        break;
                }
            }
            else
            {
                System.Windows.MessageBox.Show("Connection to the server has been lost. Returning...");
                Kotkowo kotkowo = new Kotkowo();
                kotkowo.Show();
                this.Close();
                return;
            }
        }

        public void UpdateFileList()
        {
            string[] lines;

            if (CheckConnectionStatus() == 1)
            {
                switch (GetFileList())
                {
                    case 1:
                        System.Windows.MessageBox.Show("Filelist received successfully. Analyzing...");
                        lines = System.IO.File.ReadAllLines("catlist.txt");

                        lstBox.Items.Clear();

                        foreach (string line in lines)
                        {
                            lstBox.Items.Add(line);
                        }
                        break;
                    case -1:
                        System.Windows.MessageBox.Show("Error: Filelist on the server is empty. Aborting...");
                        TerminateClientConnection();
                        System.Environment.Exit(1);
                        break;
                    case -2:
                        System.Windows.MessageBox.Show("Error: Failed to receive filelist size. Aborting...");
                        TerminateClientConnection();
                        System.Environment.Exit(1);
                        break;
                    case -9:
                        System.Windows.MessageBox.Show("Error: Unknown error has occured. Aborting...");
                        TerminateClientConnection();
                        System.Environment.Exit(1);
                        break;
                }
            }
            else
            {
                System.Windows.MessageBox.Show("Connection to the server has been lost. Returning...");
                Kotkowo kotkowo = new Kotkowo();
                kotkowo.Show();
                this.Close();
                return;
            }
        }

        public string GetPath(string name)
        {
            return System.IO.Path.GetFullPath(name);
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            if (CheckConnectionStatus() == -1)
            {
                System.Windows.MessageBox.Show("Connection has not been established yet");
                return;
            }
            else
            {
                TerminateClientConnection();
                System.Windows.MessageBox.Show("Disconnected successfully");
                Kotkowo kotkowo = new Kotkowo();
                kotkowo.Show();
                this.Close();
                return;
            }
        }

        private void UploadCat_Click(object sender, RoutedEventArgs e)
        {
            char[] fileName = fileToUpload.Text.ToCharArray();
            long fileName_length = fileToUpload.Text.Length;

            int result = UploadImage(fileName, fileName_length);
            System.Windows.MessageBox.Show("Received code: " + Convert.ToString(result));

            if (result == 1)
            {
                System.Windows.MessageBox.Show("Image sent successfully");

                if (GetFileList() == 1)
                {
                    UpdateFileList();
                    System.Windows.MessageBox.Show("Filelist updated successfully");
                }
                else
                {
                    System.Windows.MessageBox.Show("Error while updating filelist");
                }
            }
            else
            {
                System.Windows.MessageBox.Show("Error while sending image");
            }
            return;
        }

        private void CheckConnection_Click(object sender, RoutedEventArgs e)
        {
            if (CheckConnectionStatus() == 1)
            {
                System.Windows.MessageBox.Show("Connected");
            }
            else
            {
                System.Windows.MessageBox.Show("No connection established");
            }
            return;
        }

        private void DownloadCat_Click(object sender, RoutedEventArgs e)
        {
            if (lstBox.SelectedItem == null)
            {
                System.Windows.MessageBox.Show("You have to select an image name first");
                return;
            }

            string selectedImage = lstBox.SelectedItem.ToString();

            if (selectedImage == null)
            {
                System.Windows.MessageBox.Show("Please select an image name from the list before attempting to download");
                return;
            }

            if (File.Exists(selectedImage))
            {
                System.Windows.MessageBox.Show("Selected file has already been downloaded");
                return;
            }

            long nameLength = selectedImage.Length;
            char[] image2Name = selectedImage.ToCharArray();

            switch (DownloadImage(image2Name, nameLength))
            {
                case (1):
                    System.Windows.MessageBox.Show("Image downloaded successfully");
                    break;
                case (-1):
                    System.Windows.MessageBox.Show("Server could not acquire file information");
                    break;
                case (-2):
                    System.Windows.MessageBox.Show("Requested file is empty on the server");
                    break;
                case (-9):
                    System.Windows.MessageBox.Show("Error while sending file");
                    break;
                case (-10):
                    System.Windows.MessageBox.Show("Error while creating file");
                    break;
                default:
                    System.Windows.MessageBox.Show("Unknown error has occurred");
                    break;
            }
        }


        private void DisplayButton_Click(object sender, RoutedEventArgs e)
        {
            if (lstBox.SelectedItem == null)
            {
                System.Windows.MessageBox.Show("You have to select an image name first");
                return;
            }

            string selectedImage = lstBox.SelectedItem.ToString();

            if (selectedImage == null)
            {
                System.Windows.MessageBox.Show("Please select an image name from the list before attempting to display");
                return;
            }

            if (!File.Exists(selectedImage))
            {
                System.Windows.MessageBox.Show("Selected file has not been downloaded yet. Cannot display");
                return;
            }

            try
            {
                Picture.Source = (ImageSource)new ImageSourceConverter().ConvertFromString(GetPath(selectedImage));
                return;
            }
            catch (ArgumentException)
            {
                System.Windows.MessageBox.Show("File does not exist on client side. Please download it first");
            }
        }

        private void RefreshList_Click(object sender, RoutedEventArgs e)
        {
            UpdateFileList();
        }
    }
}
