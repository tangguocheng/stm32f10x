namespace ModbusTCPUpdate
{
        partial class Form1
        {
                /// <summary>
                /// 必需的设计器变量。
                /// </summary>
                private System.ComponentModel.IContainer components = null;

                /// <summary>
                /// 清理所有正在使用的资源。
                /// </summary>
                /// <param name="disposing">如果应释放托管资源，为 true；否则为 false。</param>
                protected override void Dispose(bool disposing)
                {
                        if (disposing && (components != null))
                        {
                                components.Dispose();
                        }
                        base.Dispose(disposing);
                }

                #region Windows 窗体设计器生成的代码

                /// <summary>
                /// 设计器支持所需的方法 - 不要修改
                /// 使用代码编辑器修改此方法的内容。
                /// </summary>
                private void InitializeComponent()
                {
            this.btx_open_server = new System.Windows.Forms.Button();
            this.function_area = new System.Windows.Forms.TabControl();
            this.tab_update = new System.Windows.Forms.TabPage();
            this.pgb_update_status = new System.Windows.Forms.ProgressBar();
            this.btn_update_start = new System.Windows.Forms.Button();
            this.btn_open_file = new System.Windows.Forms.Button();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            this.txb_info = new System.Windows.Forms.RichTextBox();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.lb_tcp_client = new System.Windows.Forms.ListBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.txb_server_port = new System.Windows.Forms.TextBox();
            this.txb_server_ip = new System.Windows.Forms.TextBox();
            this.btn_device_reboot = new System.Windows.Forms.Button();
            this.function_area.SuspendLayout();
            this.tab_update.SuspendLayout();
            this.groupBox4.SuspendLayout();
            this.groupBox3.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // btx_open_server
            // 
            this.btx_open_server.Location = new System.Drawing.Point(299, 21);
            this.btx_open_server.Name = "btx_open_server";
            this.btx_open_server.Size = new System.Drawing.Size(75, 23);
            this.btx_open_server.TabIndex = 0;
            this.btx_open_server.Text = "open";
            this.btx_open_server.UseVisualStyleBackColor = true;
            this.btx_open_server.Click += new System.EventHandler(this.btx_open_server_Click);
            // 
            // function_area
            // 
            this.function_area.Controls.Add(this.tab_update);
            this.function_area.Dock = System.Windows.Forms.DockStyle.Fill;
            this.function_area.Location = new System.Drawing.Point(0, 0);
            this.function_area.Multiline = true;
            this.function_area.Name = "function_area";
            this.function_area.SelectedIndex = 0;
            this.function_area.Size = new System.Drawing.Size(919, 269);
            this.function_area.TabIndex = 3;
            // 
            // tab_update
            // 
            this.tab_update.Controls.Add(this.btn_device_reboot);
            this.tab_update.Controls.Add(this.pgb_update_status);
            this.tab_update.Controls.Add(this.btn_update_start);
            this.tab_update.Controls.Add(this.btn_open_file);
            this.tab_update.Controls.Add(this.groupBox4);
            this.tab_update.Controls.Add(this.groupBox3);
            this.tab_update.Location = new System.Drawing.Point(4, 22);
            this.tab_update.Name = "tab_update";
            this.tab_update.Padding = new System.Windows.Forms.Padding(3);
            this.tab_update.Size = new System.Drawing.Size(911, 243);
            this.tab_update.TabIndex = 1;
            this.tab_update.Text = "固件更新";
            this.tab_update.UseVisualStyleBackColor = true;
            // 
            // pgb_update_status
            // 
            this.pgb_update_status.Location = new System.Drawing.Point(258, 203);
            this.pgb_update_status.Maximum = 1000;
            this.pgb_update_status.Name = "pgb_update_status";
            this.pgb_update_status.Size = new System.Drawing.Size(650, 23);
            this.pgb_update_status.Step = 1;
            this.pgb_update_status.Style = System.Windows.Forms.ProgressBarStyle.Continuous;
            this.pgb_update_status.TabIndex = 7;
            // 
            // btn_update_start
            // 
            this.btn_update_start.Location = new System.Drawing.Point(95, 203);
            this.btn_update_start.Name = "btn_update_start";
            this.btn_update_start.Size = new System.Drawing.Size(75, 23);
            this.btn_update_start.TabIndex = 6;
            this.btn_update_start.Text = "开始升级";
            this.btn_update_start.UseVisualStyleBackColor = true;
            this.btn_update_start.Click += new System.EventHandler(this.btn_update_start_Click);
            // 
            // btn_open_file
            // 
            this.btn_open_file.Location = new System.Drawing.Point(14, 203);
            this.btn_open_file.Name = "btn_open_file";
            this.btn_open_file.Size = new System.Drawing.Size(75, 23);
            this.btn_open_file.TabIndex = 5;
            this.btn_open_file.Text = "打开文件";
            this.btn_open_file.UseVisualStyleBackColor = true;
            this.btn_open_file.Click += new System.EventHandler(this.btn_open_file_Click);
            // 
            // groupBox4
            // 
            this.groupBox4.Controls.Add(this.txb_info);
            this.groupBox4.Location = new System.Drawing.Point(391, 6);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Size = new System.Drawing.Size(517, 186);
            this.groupBox4.TabIndex = 4;
            this.groupBox4.TabStop = false;
            this.groupBox4.Text = "消息列表";
            // 
            // txb_info
            // 
            this.txb_info.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.txb_info.Dock = System.Windows.Forms.DockStyle.Fill;
            this.txb_info.Location = new System.Drawing.Point(3, 17);
            this.txb_info.Name = "txb_info";
            this.txb_info.ReadOnly = true;
            this.txb_info.Size = new System.Drawing.Size(511, 166);
            this.txb_info.TabIndex = 2;
            this.txb_info.Text = "";
            this.txb_info.TextChanged += new System.EventHandler(this.txb_info_TextChanged);
            this.txb_info.DoubleClick += new System.EventHandler(this.txb_info_DoubleClick);
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.groupBox2);
            this.groupBox3.Controls.Add(this.groupBox1);
            this.groupBox3.Location = new System.Drawing.Point(6, 6);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(382, 186);
            this.groupBox3.TabIndex = 2;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "配置信息";
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.lb_tcp_client);
            this.groupBox2.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.groupBox2.Location = new System.Drawing.Point(3, 83);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(376, 100);
            this.groupBox2.TabIndex = 5;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "已连接客户端列表";
            // 
            // lb_tcp_client
            // 
            this.lb_tcp_client.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lb_tcp_client.FormattingEnabled = true;
            this.lb_tcp_client.ItemHeight = 12;
            this.lb_tcp_client.Location = new System.Drawing.Point(3, 17);
            this.lb_tcp_client.Name = "lb_tcp_client";
            this.lb_tcp_client.Size = new System.Drawing.Size(370, 80);
            this.lb_tcp_client.TabIndex = 1;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.btx_open_server);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Controls.Add(this.txb_server_port);
            this.groupBox1.Controls.Add(this.txb_server_ip);
            this.groupBox1.Dock = System.Windows.Forms.DockStyle.Top;
            this.groupBox1.Location = new System.Drawing.Point(3, 17);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(376, 62);
            this.groupBox1.TabIndex = 4;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "服务器";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(147, 28);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(41, 12);
            this.label2.TabIndex = 5;
            this.label2.Text = "端口：";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(6, 28);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(29, 12);
            this.label1.TabIndex = 4;
            this.label1.Text = "IP：";
            // 
            // txb_server_port
            // 
            this.txb_server_port.Location = new System.Drawing.Point(194, 23);
            this.txb_server_port.Name = "txb_server_port";
            this.txb_server_port.Size = new System.Drawing.Size(100, 21);
            this.txb_server_port.TabIndex = 3;
            this.txb_server_port.Text = "502";
            // 
            // txb_server_ip
            // 
            this.txb_server_ip.Location = new System.Drawing.Point(41, 23);
            this.txb_server_ip.Name = "txb_server_ip";
            this.txb_server_ip.Size = new System.Drawing.Size(100, 21);
            this.txb_server_ip.TabIndex = 2;
            this.txb_server_ip.Text = "127.0.0.1";
            // 
            // btn_device_reboot
            // 
            this.btn_device_reboot.Location = new System.Drawing.Point(177, 203);
            this.btn_device_reboot.Name = "btn_device_reboot";
            this.btn_device_reboot.Size = new System.Drawing.Size(75, 23);
            this.btn_device_reboot.TabIndex = 8;
            this.btn_device_reboot.Text = "重启设备";
            this.btn_device_reboot.UseVisualStyleBackColor = true;
            this.btn_device_reboot.Click += new System.EventHandler(this.btn_device_reboot_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(919, 269);
            this.Controls.Add(this.function_area);
            this.Name = "Form1";
            this.Text = "Form1";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            this.Load += new System.EventHandler(this.Form1_Load);
            this.function_area.ResumeLayout(false);
            this.tab_update.ResumeLayout(false);
            this.groupBox4.ResumeLayout(false);
            this.groupBox3.ResumeLayout(false);
            this.groupBox2.ResumeLayout(false);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);

                }

                #endregion

                private System.Windows.Forms.Button btx_open_server;
                private System.Windows.Forms.TabControl function_area;
                private System.Windows.Forms.TabPage tab_update;
                private System.Windows.Forms.RichTextBox txb_info;
        private System.Windows.Forms.ListBox lb_tcp_client;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox txb_server_port;
        private System.Windows.Forms.TextBox txb_server_ip;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.Button btn_open_file;
        private System.Windows.Forms.ProgressBar pgb_update_status;
        private System.Windows.Forms.Button btn_update_start;
        private System.Windows.Forms.Button btn_device_reboot;
    }
}

