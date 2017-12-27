using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
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
                modbus_client.ConnectClose();
            }
        }

        private void start_tcp_server()
        {
            string ip = "192.168.0.106";
            IPAddress ipaddr = IPAddress.Parse(ip);
            TcpListener listener = new TcpListener(ipaddr, 5000);
            listener.Start(10);
            while (true)
            {
                TcpClient client = listener.AcceptTcpClient();
                string client_ip = client.Client.RemoteEndPoint.ToString().Split(':')[0];
                if (!lb_modbus_slave.Items.Contains(client_ip))
                    lb_modbus_slave.Items.Add(client_ip);
            }
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            Control.CheckForIllegalCrossThreadCalls = false;
            new Thread(start_tcp_server).Start();
        }

        private ModBusTcpClient modbus_client;
        private bool isPortOpen = false;
        private void btx_open_server_Click(object sender, EventArgs e)
        {
            if (!isPortOpen)
            {
                modbus_client = new ModBusTcpClient(lb_modbus_slave.SelectedItem.ToString(),502);
                modbus_client.ConnectServer();
                btx_open_server.Text = "close";
            }

        }

        private void TcpServer_OnDataReceived(byte[] object1)
        {
            BeginInvoke(new Action<byte[]>(ShowModbusData), object1);
        }

        private void ShowModbusData(byte[] modbus)
        {
            string transaction_id = BitConverter.ToString(modbus, 0, 2);
            string protocol_id = BitConverter.ToString(modbus, 2, 2); ;
            string length = BitConverter.ToString(modbus, 4, 1); ;
            string unit_id = BitConverter.ToString(modbus, 5, 1); ;
            string Fcode = BitConverter.ToString(modbus, 6, 1); ;
            string data = BitConverter.ToString(modbus, 7); ;

            txb_info.AppendText(DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss.fff") + " :" + Environment.NewLine +
                "transaction_id: " + transaction_id + Environment.NewLine +
                "protocol_id: " + protocol_id + Environment.NewLine +
                "length: " + length + Environment.NewLine +
                "unit_id: " + unit_id + Environment.NewLine +
                "Fcode: " + Fcode + Environment.NewLine +
                "data: " + data + Environment.NewLine);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            byte[] data = { 1,21,34};
            modbus_client.WriteRegister(0,1234567);
            check_passwd();
        }

        private bool check_passwd()
        {
            byte[] data = HslCommunication.BasicFramework.SoftBasic.HexStringToBytes("00 00 00 00 00 0B FF 65 FF F0 00 02 04 00 00 00 00");

            HslCommunication.OperateResult<byte[]> read = modbus_client.ReadFromServerCore(data);

            if (read.IsSuccess)
            {
                // 获取结果，并转化为Hex字符串，方便显示
                string result = HslCommunication.BasicFramework.SoftBasic.ByteToHexString(read.Content, ' ');
                txb_info.AppendText(result);
                if (read.Content[7] == 0x65 && read.Content[8] == 0x01) { }
            }
            else
            {
                MessageBox.Show(read.ToMessageShowString());
            }

            return true;
        }
    }
}
