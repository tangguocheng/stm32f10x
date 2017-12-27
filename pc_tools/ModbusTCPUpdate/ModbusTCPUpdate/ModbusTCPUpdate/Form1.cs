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

                ~Form1()
                {
                        if (isPortOpen)
                        {
                                tcpServer.ServerClose();
                        }
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
                                isPortOpen = true;
                                btx_open_server.Text = "close";
                        }

                }

                private void TcpServer_OnDataReceived(byte[] object1)
                {
                        BeginInvoke(new Action<byte[]>(ShowModbusData), object1);
                }

                private void ShowModbusData(byte[] modbus)
                {
                        string transaction_id = BitConverter.ToString(modbus,0,2);
                        string protocol_id = BitConverter.ToString(modbus, 2, 2); ;
                        string length = BitConverter.ToString(modbus, 4, 1); ;
                        string unit_id = BitConverter.ToString(modbus, 5, 1); ;
                        string Fcode = BitConverter.ToString(modbus,6, 1); ;
                        string data = BitConverter.ToString(modbus, 7); ;

                        txb_info.AppendText(DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss.fff") + " :" + Environment.NewLine+
                            "transaction_id: " + transaction_id + Environment.NewLine +
                            "protocol_id: " + protocol_id + Environment.NewLine +
                            "length: " + length + Environment.NewLine +
                            "unit_id: " + unit_id + Environment.NewLine +
                            "Fcode: " + Fcode + Environment.NewLine +
                            "data: " + data + Environment.NewLine);
                }

                private void button1_Click(object sender, EventArgs e)
                {
                        tcpServer.WriteRegister(0, 12);
                        byte[] test = tcpServer.ReadRegister(0, 1);
                        txb_info.AppendText(BitConverter.ToString(test));
                }
        }
}
