using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using HslCommunication.ModBus;

namespace ModbusTCPUpdate
{
        public partial class Form1 : Form
        {
                public Form1()
                {
                        InitializeComponent();
                }

                private void Form1_Load(object sender, EventArgs e)
                {

                }
                private ModBusTcpServer tcpServer;
                private bool isPortOpen = false;
                private void btx_open_server_Click(object sender, EventArgs e)
                {
                        if (!isPortOpen)
                        {
                                tcpServer = new ModBusTcpServer();
                                tcpServer.LogNet = new HslCommunication.LogNet.LogNetSingle(Application.StartupPath + @"\Logs\log.txt");
                                tcpServer.OnDataReceived += TcpServer_OnDataReceived;
                                tcpServer.ServerStart(502);
                        }

                }

                private void TcpServer_OnDataReceived(byte[] object1)
                {
                        BeginInvoke(new Action<byte[]>(ShowModbusData), object1);
                }


                private void ShowModbusData(byte[] modbus)
                {
                        textBox1.AppendText(DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss.fff") + " :" +
                            HslCommunication.BasicFramework.SoftBasic.ByteToHexString(modbus) + Environment.NewLine);
                }
        }
}
