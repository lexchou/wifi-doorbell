using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Doorbell
{
    class Program
    {
        const string Server = "10.0.0.1";
        const int Port = 3001;
        NotifyIcon icon;
        Socket socket;
        NetworkStream stream;
        byte[] buffer = new byte[20];
        void Run()
        {
            InitUI();
            InitNetwork();

            Application.Run();
        }
        private void InitNetwork()
        {
            socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            socket.Connect(Server, Port);
            socket.Blocking = false;
            ScheduleNextRead();

        }
        private void ScheduleNextRead()
        {
            socket.BeginReceive(buffer, 0, buffer.Length,
                SocketFlags.None, delegate (IAsyncResult ar)
                {
                    int bytes = socket.EndReceive(ar);
                    string command = Encoding.UTF8.GetString(buffer, 0, bytes).Trim();
                    if("RING".Equals(command))
                    {
                        Ring();
                    }
                    ScheduleNextRead();
                }, null);
        }
        private void Ring()
        {
            icon.ShowBalloonTip(10 * 1000, "门铃", "有人在敲门", ToolTipIcon.Info);
        }
        private void InitUI()
        {

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            icon = new NotifyIcon();
            icon.Icon = SystemIcons.Information;
            icon.Text = "WiFi门铃";
            icon.Visible = true;
            ContextMenu menu = new ContextMenu();
            menu.MenuItems.Add("退出").Click += OnQuit;
            icon.ContextMenu = menu;

        }


        /// <summary>
        /// 应用程序的主入口点。
        /// </summary>
        [STAThread]
        static void Main()
        {
            (new Program()).Run();
        }

        private void OnQuit(object sender, EventArgs e)
        {
            icon.Visible = false;
            Application.Exit();
        }
    }
}
