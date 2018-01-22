using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
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
                serverSocket.Close();
            }

            if (tcp_listen_thread != null && tcp_listen_thread.IsAlive) tcp_listen_thread.Abort();
            if (tcp_client_receive != null && tcp_client_receive.IsAlive) tcp_client_receive.Abort();
            if (tcp_server_send != null && tcp_server_send.IsAlive) tcp_server_send.Abort();
            if (device_update_thread != null && device_update_thread.IsAlive) device_update_thread.Abort();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            Control.CheckForIllegalCrossThreadCalls = false;
        }

        private List<Socket> client_socket = new List<Socket>();

        private void add_infomation(string info)
        {
            txb_info.AppendText("[ " + System.DateTime.Now.TimeOfDay.ToString() + " ] " + info + "\n");
        }

        private void txb_info_TextChanged(object sender, EventArgs e)
        {
            txb_info.Focus();
        }

        private void txb_info_DoubleClick(object sender, EventArgs e)
        {
            txb_info.Clear();
        }

        private bool isPortOpen = false;
        private Socket serverSocket;
        private Thread tcp_listen_thread = null;
        private Thread tcp_client_receive = null;
        private Thread tcp_server_send = null;
        private Thread device_update_thread = null;
        private void btx_open_server_Click(object sender, EventArgs e)
        {
            string ip = txb_server_ip.Text;
            int myport = Convert.ToInt32(txb_server_port.Text);
            if (!isPortOpen)
            {
                IPAddress server_ip = IPAddress.Parse(ip);
                serverSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                serverSocket.Bind(new IPEndPoint(server_ip, myport));
                serverSocket.Listen(10);
                add_infomation("监听：" + ip + ":" + myport.ToString());

                btx_open_server.Text = "关闭";
                isPortOpen = true;

                tcp_listen_thread = new Thread(start_server_listen);
                tcp_listen_thread.Start();
            }
            else
            {
                serverSocket.Close();
                add_infomation("关闭：" + ip + ":" + myport.ToString());
                btx_open_server.Text = "打开";
                isPortOpen = false;
            }

        }

        private void start_server_listen()
        {
            while (true)
            {
                try
                {
                    Socket client = serverSocket.Accept();
                    string client_ip = client.RemoteEndPoint.ToString();

                    add_infomation("客户端：" + client_ip + " 已连接。");

                    //if (!lb_tcp_client.Items.Contains(client_ip))
                    {
                        lb_tcp_client.Items.Add(client_ip);
                        client_socket.Add(client);
                    }

                    foreach (Socket clt in client_socket)
                    {

                    }
                }
                catch (Exception e)
                {
                    add_infomation(e.Message);
                    return;
                }
            }
        }

        private bool check_passwd()
        {
            byte[] data = HslCommunication.BasicFramework.SoftBasic.HexStringToBytes("00 00 00 00 00 0B FF 65 FF F0 00 02 04 00 00 00 00");

            return true;
        }

        private OpenFileDialog open_file_dialog;

        private void btn_open_file_Click(object sender, EventArgs e)
        {
            open_file_dialog = new OpenFileDialog();
            open_file_dialog.FileOk += Open_file_dialog_FileOk;
            open_file_dialog.DefaultExt = "bin";
            open_file_dialog.Filter = "Bin files (*.bin)|*.bin";
            open_file_dialog.Title = "选择控制板升级文件";
            open_file_dialog.ShowDialog();
        }

        BinaryReader bw;
        private string bin_file_name = null;
        private void Open_file_dialog_FileOk(object sender, CancelEventArgs e)
        {
            OpenFileDialog file_dialog = (OpenFileDialog)(sender);
            add_infomation("打开文件：" + file_dialog.FileName);
            bin_file_name = file_dialog.FileName;
        }

        Socket wait_update_clinet = null;

        private void btn_update_start_Click(object sender, EventArgs e)
        {
            if (lb_tcp_client.SelectedItem == null)
            {
                add_infomation("请在设备列表中选择需要升级的设备！");
            }
            else
            {
                wait_update_clinet = client_socket[lb_tcp_client.SelectedIndex];
            }

            if (wait_update_clinet != null)
            {
                add_infomation("选择设备：" + wait_update_clinet.RemoteEndPoint.ToString());
                device_update_thread = new Thread(update_device);
                device_update_thread.Start(bin_file_name);
                btn_update_start.Enabled = false;
            }
        }


        private UInt16 update_count = 0;
        private UInt16 last_addr = 0;
        private bool sent_update_to_device(byte[] data)
        {
            byte[] mb = new byte[data.Length + 12];

            mb[0] = (byte)((update_count >> 8) & 0xFF);
            mb[1] = (byte)(update_count & 0xFF);

            update_count++;

            mb[2] = 0x00;
            mb[3] = 0x00;

            UInt16 pdu_len = (UInt16)(1 + 5 + data.Length);
            mb[4] = (byte)((pdu_len >> 8) & 0xFF);
            mb[5] = (byte)(pdu_len & 0xFF);
            mb[6] = 0xFF;

            mb[7] = 0x66;

            UInt16 addr = last_addr;

            mb[8] = (byte)(addr & 0xFF);
            mb[9] = (byte)((addr >> 8) & 0xFF);

            mb[10] = (byte)(data.Length & 0xFF);
            mb[11] = (byte)((data.Length >> 8) & 0xFF);
            

            for (int i = 0; i < data.Length; i++)
            {
                mb[12 + i] = data[i];
            }

            bool result = true;
            try
            {
                wait_update_clinet.Send(mb);
            }
            catch (Exception e)
            {
                result = false;
                add_infomation("发送数据失败：" + e.Message);
            }

            try
            {
                wait_update_clinet.ReceiveTimeout = 3000;

                byte[] receive_data = new byte[1024];
                wait_update_clinet.Receive(receive_data);
                //Todo: 检查从机返回数据
                UInt16 length_echo = (UInt16)(((receive_data[11] << 8) & 0xFF00) | receive_data[10]);
                if ((receive_data[7] == 0x66) && (length_echo == data.Length))
                {
                    last_addr = (UInt16)(last_addr + data.Length);
                    result = true;
                }
                else
                {
                    result = false;
                }
            }
            catch (Exception e)
            {
                result = false;
                add_infomation("接收数据失败：" + e.Message);
            }

            return result;
        }

        private bool send_password()
        {
            byte[] mb = new byte[14];

            mb[0] = (byte)((update_count >> 8) & 0xFF);
            mb[1] = (byte)(update_count & 0xFF);

            update_count++;

            mb[2] = 0x00;
            mb[3] = 0x00;

            UInt16 pdu_len = (UInt16)(1 + 7);
            mb[4] = (byte)((pdu_len >> 8) & 0xFF);
            mb[5] = (byte)(pdu_len & 0xFF);
            mb[6] = 0x01;

            mb[7] = 0x65;
            mb[8] = 0xF0;
            mb[9] = 0xFF;
            mb[10] = 0x66;
            mb[11] = 0x66;
            mb[12] = 0x66;
            mb[13] = 0x66;

            bool result = true;
            try
            {
                wait_update_clinet.Send(mb);
            }
            catch (Exception e)
            {
                result = false;
                add_infomation("发送数据失败：" + e.Message);
            }

            try
            {
                wait_update_clinet.ReceiveTimeout = 3000;

                byte[] receive_data = new byte[1024];
                wait_update_clinet.Receive(receive_data);
                //Todo: 检查从机返回数据
                UInt16 length_echo = (UInt16)(((receive_data[10] << 8) & 0xFF00) | receive_data[11]);
                if ((receive_data[7] == 0x65) && (receive_data[8] == 0x01))
                    result = true;
                else
                    result = false;
            }
            catch (Exception e)
            {
                result = false;
                add_infomation("接收数据失败：" + e.Message);
            }
            return result;
        }

        private bool send_update_enable()
        {
            byte[] mb = new byte[7 + 5];

            mb[0] = (byte)((update_count >> 8) & 0xFF);
            mb[1] = (byte)(update_count & 0xFF);

            update_count++;

            mb[2] = 0x00;
            mb[3] = 0x00;

            UInt16 pdu_len = (UInt16)(1 + 5);
            mb[4] = (byte)((pdu_len >> 8) & 0xFF);
            mb[5] = (byte)(pdu_len & 0xFF);
            mb[6] = 0x01;

            mb[7] = 0x67;
            mb[8] = 0x01;
            mb[9] = 0xFF;
            mb[10] = 0x66;
            mb[10] = 0x66;

            bool result = true;
            try
            {
                wait_update_clinet.Send(mb);
            }
            catch (Exception e)
            {
                result = false;
                add_infomation("发送数据失败：" + e.Message);
            }

            try
            {
                wait_update_clinet.ReceiveTimeout = 3000;

                byte[] receive_data = new byte[1024];
                wait_update_clinet.Receive(receive_data);
                //Todo: 检查从机返回数据
                if (receive_data[7] == 0x67)
                {
                    add_infomation("当前软件版本：V" + receive_data[7].ToString() + "." + receive_data[8].ToString()
                                     + "." + receive_data[9].ToString());
                    result = true;
                }
                else
                    result = false;

            }
            catch (Exception e)
            {
                result = false;
                add_infomation("接收数据失败：" + e.Message);
            }
            return result;
        }

        private bool send_update_done()
        {
            byte[] mb = new byte[7 + 2];

            mb[0] = (byte)((update_count >> 8) & 0xFF);
            mb[1] = (byte)(update_count & 0xFF);

            update_count++;

            mb[2] = 0x00;
            mb[3] = 0x00;

            UInt16 pdu_len = (UInt16)(1 + 5);
            mb[4] = (byte)((pdu_len >> 8) & 0xFF);
            mb[5] = (byte)(pdu_len & 0xFF);
            mb[6] = 0x01;

            mb[7] = 0x67;
            mb[8] = 0x02;

            bool result = true;
            try
            {
                wait_update_clinet.Send(mb);
            }
            catch (Exception e)
            {
                result = false;
                add_infomation("发送数据失败：" + e.Message);
            }

            try
            {
                wait_update_clinet.ReceiveTimeout = 3000;

                byte[] receive_data = new byte[1024];
                wait_update_clinet.Receive(receive_data);
                //Todo: 检查从机返回数据
                if (receive_data[7] == 0x67)
                {
                    add_infomation("结束升级");
                    result = true;
                }
                else
                    result = false;

            }
            catch (Exception e)
            {
                result = false;
                add_infomation("接收数据失败：" + e.Message);
            }
            return result;
        }
        private bool reboot_device()
        {
            if (lb_tcp_client.SelectedItem == null)
            {
                add_infomation("请在设备列表中选择需要重启的设备！");
                return false;
            }
            else
            {
                wait_update_clinet = client_socket[lb_tcp_client.SelectedIndex];
            }

            add_infomation("选择设备：" + wait_update_clinet.RemoteEndPoint.ToString());

            byte[] mb = new byte[7 + 1];

            mb[0] = (byte)((update_count >> 8) & 0xFF);
            mb[1] = (byte)(update_count & 0xFF);

            update_count++;

            mb[2] = 0x00;
            mb[3] = 0x00;

            UInt16 pdu_len = (UInt16)(1 + 1);
            mb[4] = (byte)((pdu_len >> 8) & 0xFF);
            mb[5] = (byte)(pdu_len & 0xFF);
            mb[6] = 0x01;

            mb[7] = 0x68;

            add_infomation("重启设备...");

            bool rtl = true;

            try
            {
                wait_update_clinet.Send(mb);
            }
            catch (Exception ex)
            {
                add_infomation("重启失败：" + ex.Message);
                rtl = false;
            }

            if (rtl)
                add_infomation("重启成功，等待设备重连..." );

            return (rtl);
        }

        private bool isUpdating = false;
        private void update_device(object bin_file_path)
        {
            string file_path = (string)bin_file_path;
            System.DateTime startTime;
            if (file_path != null)
            {
                add_infomation("开始升级：" + file_path);
                startTime = System.DateTime.Now;
                isUpdating = true;
                bw = new BinaryReader(new FileStream(file_path, FileMode.Open));
                bool update_end = false;
                bool get_file_size = true;
                long file_size = 0;
                while (get_file_size)
                {
                    byte[] bin_data = bw.ReadBytes(1024);
                    file_size += bin_data.Length;

                    if (bin_data.Length < 1024)
                        get_file_size = false;
                }

                add_infomation("文件大小：" + file_size.ToString() + " byte");

                bw.Close();
                bw = new BinaryReader(new FileStream(file_path, FileMode.Open));

                long remain_data_size = file_size;
                add_infomation("验证密码...");

                if (send_password())
                {
                    update_end = false;
                    add_infomation("密码验证通过...");
                }
                else
                {
                    update_end = true;
                    add_infomation("密码验证失败...");
                }

                add_infomation("使能升级...");
                
                if (send_update_enable())
                {
                    update_end = false;
                    add_infomation("升级使能成功...");
                }
                else
                {
                    update_end = true;
                    add_infomation("升级使能失败...");
                }
                
                add_infomation("开始升级，请勿断电或重启设备及软件！");

                bool result = false;
                last_addr = 0;
                while (update_end == false)
                {
                    byte[] bin_data = bw.ReadBytes(256);

                    string bin_data_str = BitConverter.ToString(bin_data);

                    if (bin_data.Length < 256)      //reach file end
                    {
                        update_end = true;
                    }

                    remain_data_size -= bin_data.Length;

                    int percent = 1000 - (int)(remain_data_size * 1.0 / file_size * 1000);

                    pgb_update_status.Value = percent;

                    int retry_cnt = 0;

                    result = false;
                    while ((result == false) && (retry_cnt++ < 3))
                    {
                        result = sent_update_to_device(bin_data);
                        if (result == false)
                            add_infomation("接收超时，第" + retry_cnt.ToString() + "次重试");
                    }

                    if (result == false)
                    {
                        break;
                    }

                    Thread.Sleep(100);
                }

                bw.Close();
                isUpdating = false;
                btn_update_start.Enabled = true;
                if (result && send_update_done())
                {
                    update_end = false;
                    add_infomation("升级完成，耗时：" + (System.DateTime.Now - startTime).ToString() + ",线程退出...");

                    if (ckb_auto_reboot.Checked)
                        reboot_device();
                }
                else
                {
                    pgb_update_status.Value = 0;
                    add_infomation("升级失败，线程退出...");
                }
            }
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (isUpdating)
            {
                MessageBox.Show("设备正在升级，请等待升级完成！");
                e.Cancel = true;
            }
        }

        private void btn_device_reboot_Click(object sender, EventArgs e)
        {
            reboot_device();
        }

    }
}