using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Runtime.InteropServices;
using System.Reflection;
using System.IO;


namespace ClientCSharp
{
  public partial class Form1 : Form
  {
    internal Socket Client { get; set; }
    internal IPAddress Ip { get; set; }
    internal Int32 Port { get; set; }

    internal System.Threading.Timer Timer { get; set; }

    internal Thread thread;

    private bool _connected = false;
    private int _height;

    #region external func
    [DllImport("kernel32", CharSet = CharSet.Unicode)]
    private static extern int GetPrivateProfileString(string Section, string Key, string Default, StringBuilder RetVal, int Size, string FilePath);
    #endregion

    public Form1()
    {
      InitializeComponent();
      _height = this.Height;
    }

    private void LoadSettings()
    {
      int rs = 0;
      var cb = Assembly.GetExecutingAssembly().CodeBase;
      string filePath = new Uri(cb).LocalPath;

      try {
        string path = Path.Combine(Path.GetDirectoryName(filePath), "config.ini");
        var RetVal = new StringBuilder(255);
        rs = GetPrivateProfileString("Connection", "Server", "127.0.0.1", RetVal, 255, path);
        Ip = IPAddress.Parse(RetVal.ToString());
        rs = GetPrivateProfileString("Connection", "Port", "9990", RetVal, 255, path);
        Port = Int32.Parse(RetVal.ToString());
      }
      catch( Exception ) {
        Ip = new IPAddress(new byte[] { 127, 0, 0, 1 });
        Port = 9990;
      }
    }

    private void SendMessage(string Msg)
    {
      if( !String.IsNullOrWhiteSpace(Msg) ) {
        try {
          byte[] buffer = new byte[1024];
          buffer = Encoding.UTF8.GetBytes(Msg);
          Client.Send(buffer);
        }
        catch {
          _connected = false;
          txtMsg.Enabled = false;
          btnSend.Enabled = false;
        }
      }
    }
    private void RecvMessage()
    {
      byte[] buffer = new byte[1024];
      for( int i = 0; i < buffer.Length; ++i ) { buffer[i] = 0; }  // <- reset buffer

      while( true ) {
        try {
          var len = Client.Receive(buffer);
          string msg = Encoding.UTF8.GetString(buffer, 0, len);    // <- always set actual len of reciving message

          if( !String.IsNullOrWhiteSpace(msg) ) {
            Invoke((MethodInvoker)delegate() {
              txtMsg.Text = msg;
            });
          }
        }
        catch( Exception ) {
          //something going wrong....
        }
      }
    }

    public static void ConnectToServer(object obj)
    {
      Form1 frm = (Form1)obj;
      if( frm._connected == false ) {
        try {
          //LoadSettings();
          frm.Client = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
          frm.Client.Connect(frm.Ip, frm.Port);
          frm.thread = new Thread(delegate() { frm.RecvMessage(); });
          frm.thread.Start();

          frm._connected = true;
          frm.Invoke((MethodInvoker)delegate()
          {
            frm.txtMsg.Enabled = true;
            frm.btnSend.Enabled = true;
          });
        }
        catch( Exception ) {
          frm.Invoke((MethodInvoker)delegate() {
            frm.txtMsg.Enabled = false;
            frm.btnSend.Enabled = false;
          });
        }
      }
    }

    private void Form1_Load(object sender, EventArgs e)
    {
      try {
        LoadSettings();
        TimerCallback cnn = new TimerCallback(ConnectToServer);
        Timer = new System.Threading.Timer(cnn, this, 0, 5000);
      }
      catch( Exception ) {
        txtMsg.Enabled = false;
        btnSend.Enabled = false;
      }

      //try {
      //  LoadSettings();
      //  Client = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
      //  Client.Connect(Ip, Port);
      //  thread = new Thread(delegate() { RecvMessage(); });
      //  thread.Start();
      //}
      //catch(Exception) {
      //  //something going wrong....

      //  txtMsg.Enabled = false;
      //  btnSend.Enabled = false;
      //}
    }

    private void Form1_FormClosing(object sender, FormClosingEventArgs e)
    {
      if( thread != null ) {
        thread.Abort();
      }
    }

    private void btnSend_Click(object sender, EventArgs e)
    {
      SendMessage(txtMsg.Text.Trim());
    }

    protected override void OnResize(EventArgs e)
    {
      this.Height = _height;
      base.OnResize(e);
    }
  }
}
