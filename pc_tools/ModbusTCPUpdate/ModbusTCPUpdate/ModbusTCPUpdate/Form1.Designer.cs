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
                        this.textBox1 = new System.Windows.Forms.TextBox();
                        this.SuspendLayout();
                        // 
                        // btx_open_server
                        // 
                        this.btx_open_server.Location = new System.Drawing.Point(94, 152);
                        this.btx_open_server.Name = "btx_open_server";
                        this.btx_open_server.Size = new System.Drawing.Size(75, 23);
                        this.btx_open_server.TabIndex = 0;
                        this.btx_open_server.Text = "open";
                        this.btx_open_server.UseVisualStyleBackColor = true;
                        this.btx_open_server.Click += new System.EventHandler(this.btx_open_server_Click);
                        // 
                        // textBox1
                        // 
                        this.textBox1.Location = new System.Drawing.Point(12, 12);
                        this.textBox1.Name = "textBox1";
                        this.textBox1.Size = new System.Drawing.Size(372, 21);
                        this.textBox1.TabIndex = 1;
                        // 
                        // Form1
                        // 
                        this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
                        this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
                        this.ClientSize = new System.Drawing.Size(438, 262);
                        this.Controls.Add(this.textBox1);
                        this.Controls.Add(this.btx_open_server);
                        this.Name = "Form1";
                        this.Text = "Form1";
                        this.Load += new System.EventHandler(this.Form1_Load);
                        this.ResumeLayout(false);
                        this.PerformLayout();

                }

                #endregion

                private System.Windows.Forms.Button btx_open_server;
                private System.Windows.Forms.TextBox textBox1;
        }
}

