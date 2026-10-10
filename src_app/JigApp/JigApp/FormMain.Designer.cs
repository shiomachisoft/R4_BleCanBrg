
namespace JigApp
{
    partial class FormMain
    {
        /// <summary>
        /// 必要なデザイナー変数です。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 使用中のリソースをすべてクリーンアップします。
        /// </summary>
        /// <param name="disposing">マネージド リソースを破棄する場合は true を指定し、その他の場合は false を指定します。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows フォーム デザイナーで生成されたコード

        /// <summary>
        /// デザイナー サポートに必要なメソッドです。このメソッドの内容を
        /// コード エディターで変更しないでください。
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.label_DeviceUid = new System.Windows.Forms.Label();
            this.button_Connect = new System.Windows.Forms.Button();
            this.comboBox_Port = new System.Windows.Forms.ComboBox();
            this.button_Can = new System.Windows.Forms.Button();
            this.label_FwName = new System.Windows.Forms.Label();
            this.label_FwVer = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.label_ConnectStatus = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.label7 = new System.Windows.Forms.Label();
            this.label_AppVer = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.label_AppName = new System.Windows.Forms.Label();
            this.button_EraseEeprom = new System.Windows.Forms.Button();
            this.textBox_FwErr = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.timer = new System.Windows.Forms.Timer(this.components);
            this.button_ClearFwErr = new System.Windows.Forms.Button();
            this.textBox_AppLog = new System.Windows.Forms.TextBox();
            this.label9 = new System.Windows.Forms.Label();
            this.button_ClearAppLog = new System.Windows.Forms.Button();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.SuspendLayout();
            // 
            // label_DeviceUid
            // 
            this.label_DeviceUid.AutoSize = true;
            this.label_DeviceUid.Location = new System.Drawing.Point(210, 192);
            this.label_DeviceUid.Name = "label_DeviceUid";
            this.label_DeviceUid.Size = new System.Drawing.Size(43, 21);
            this.label_DeviceUid.TabIndex = 11;
            this.label_DeviceUid.Text = "---";
            // 
            // button_Connect
            // 
            this.button_Connect.Location = new System.Drawing.Point(275, 115);
            this.button_Connect.Name = "button_Connect";
            this.button_Connect.Size = new System.Drawing.Size(235, 65);
            this.button_Connect.TabIndex = 10;
            this.button_Connect.TabStop = false;
            this.button_Connect.Text = "Connect";
            this.button_Connect.UseVisualStyleBackColor = true;
            this.button_Connect.Click += new System.EventHandler(this.button_Connect_Click);
            // 
            // comboBox_Port
            // 
            this.comboBox_Port.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBox_Port.FormattingEnabled = true;
            this.comboBox_Port.Location = new System.Drawing.Point(20, 55);
            this.comboBox_Port.MaxDropDownItems = 100;
            this.comboBox_Port.Name = "comboBox_Port";
            this.comboBox_Port.Size = new System.Drawing.Size(490, 29);
            this.comboBox_Port.TabIndex = 9;
            this.comboBox_Port.TabStop = false;
            // 
            // button_Can
            // 
            this.button_Can.Location = new System.Drawing.Point(612, 67);
            this.button_Can.Name = "button_Can";
            this.button_Can.Size = new System.Drawing.Size(209, 85);
            this.button_Can.TabIndex = 18;
            this.button_Can.TabStop = false;
            this.button_Can.Text = "CAN";
            this.button_Can.UseVisualStyleBackColor = true;
            this.button_Can.Click += new System.EventHandler(this.button_Can_Click);
            // 
            // label_FwName
            // 
            this.label_FwName.AutoSize = true;
            this.label_FwName.Location = new System.Drawing.Point(210, 112);
            this.label_FwName.Name = "label_FwName";
            this.label_FwName.Size = new System.Drawing.Size(43, 21);
            this.label_FwName.TabIndex = 29;
            this.label_FwName.Text = "---";
            // 
            // label_FwVer
            // 
            this.label_FwVer.AutoSize = true;
            this.label_FwVer.Location = new System.Drawing.Point(210, 152);
            this.label_FwVer.Name = "label_FwVer";
            this.label_FwVer.Size = new System.Drawing.Size(43, 21);
            this.label_FwVer.TabIndex = 30;
            this.label_FwVer.Text = "---";
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.label_ConnectStatus);
            this.groupBox1.Controls.Add(this.button_Connect);
            this.groupBox1.Controls.Add(this.label4);
            this.groupBox1.Controls.Add(this.comboBox_Port);
            this.groupBox1.Location = new System.Drawing.Point(14, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(581, 204);
            this.groupBox1.TabIndex = 31;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Connect";
            // 
            // label_ConnectStatus
            // 
            this.label_ConnectStatus.BackColor = System.Drawing.SystemColors.Control;
            this.label_ConnectStatus.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.label_ConnectStatus.Font = new System.Drawing.Font("ＭＳ ゴシック", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(128)));
            this.label_ConnectStatus.ForeColor = System.Drawing.SystemColors.ControlText;
            this.label_ConnectStatus.Location = new System.Drawing.Point(20, 115);
            this.label_ConnectStatus.Name = "label_ConnectStatus";
            this.label_ConnectStatus.Size = new System.Drawing.Size(235, 65);
            this.label_ConnectStatus.TabIndex = 39;
            this.label_ConnectStatus.Text = "Disconnected";
            this.label_ConnectStatus.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(20, 30);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(109, 21);
            this.label4.TabIndex = 32;
            this.label4.Text = "COM Port:";
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.label7);
            this.groupBox2.Controls.Add(this.label_AppVer);
            this.groupBox2.Controls.Add(this.label5);
            this.groupBox2.Controls.Add(this.label3);
            this.groupBox2.Controls.Add(this.label2);
            this.groupBox2.Controls.Add(this.label_FwVer);
            this.groupBox2.Controls.Add(this.label1);
            this.groupBox2.Controls.Add(this.label_AppName);
            this.groupBox2.Controls.Add(this.label_FwName);
            this.groupBox2.Controls.Add(this.label_DeviceUid);
            this.groupBox2.Location = new System.Drawing.Point(14, 233);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(581, 235);
            this.groupBox2.TabIndex = 32;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "App/FW Information";
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(12, 72);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(142, 21);
            this.label7.TabIndex = 39;
            this.label7.Text = "App Version:";
            // 
            // label_AppVer
            // 
            this.label_AppVer.AutoSize = true;
            this.label_AppVer.Location = new System.Drawing.Point(210, 72);
            this.label_AppVer.Name = "label_AppVer";
            this.label_AppVer.Size = new System.Drawing.Size(43, 21);
            this.label_AppVer.TabIndex = 38;
            this.label_AppVer.Text = "---";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(12, 32);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(109, 21);
            this.label5.TabIndex = 38;
            this.label5.Text = "App Name:";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(12, 192);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(120, 21);
            this.label3.TabIndex = 35;
            this.label3.Text = "Unique ID:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 152);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(131, 21);
            this.label2.TabIndex = 34;
            this.label2.Text = "FW Version:";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 112);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(98, 21);
            this.label1.TabIndex = 33;
            this.label1.Text = "FW Name:";
            // 
            // label_AppName
            // 
            this.label_AppName.AutoSize = true;
            this.label_AppName.Location = new System.Drawing.Point(210, 32);
            this.label_AppName.Name = "label_AppName";
            this.label_AppName.Size = new System.Drawing.Size(43, 21);
            this.label_AppName.TabIndex = 32;
            this.label_AppName.Text = "---";
            // 
            // button_EraseEeprom
            // 
            this.button_EraseEeprom.Location = new System.Drawing.Point(612, 305);
            this.button_EraseEeprom.Name = "button_EraseEeprom";
            this.button_EraseEeprom.Size = new System.Drawing.Size(209, 85);
            this.button_EraseEeprom.TabIndex = 34;
            this.button_EraseEeprom.TabStop = false;
            this.button_EraseEeprom.Text = "Erase settings in EEPROM";
            this.button_EraseEeprom.UseVisualStyleBackColor = true;
            this.button_EraseEeprom.Click += new System.EventHandler(this.button_EraseEeprom_Click);
            // 
            // textBox_FwErr
            // 
            this.textBox_FwErr.Location = new System.Drawing.Point(14, 739);
            this.textBox_FwErr.Margin = new System.Windows.Forms.Padding(6, 5, 6, 5);
            this.textBox_FwErr.Multiline = true;
            this.textBox_FwErr.Name = "textBox_FwErr";
            this.textBox_FwErr.ReadOnly = true;
            this.textBox_FwErr.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.textBox_FwErr.Size = new System.Drawing.Size(581, 95);
            this.textBox_FwErr.TabIndex = 35;
            this.textBox_FwErr.TabStop = false;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(14, 714);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(109, 21);
            this.label6.TabIndex = 36;
            this.label6.Text = "FW Error:";
            // 
            // timer
            // 
            this.timer.Enabled = true;
            this.timer.Interval = 1000;
            this.timer.Tick += new System.EventHandler(this.timer_Tick);
            // 
            // button_ClearFwErr
            // 
            this.button_ClearFwErr.Location = new System.Drawing.Point(612, 758);
            this.button_ClearFwErr.Name = "button_ClearFwErr";
            this.button_ClearFwErr.Size = new System.Drawing.Size(112, 50);
            this.button_ClearFwErr.TabIndex = 37;
            this.button_ClearFwErr.TabStop = false;
            this.button_ClearFwErr.Text = "Clear";
            this.button_ClearFwErr.UseVisualStyleBackColor = true;
            this.button_ClearFwErr.Click += new System.EventHandler(this.button_ClearFwErr_Click);
            // 
            // textBox_AppLog
            // 
            this.textBox_AppLog.Location = new System.Drawing.Point(14, 509);
            this.textBox_AppLog.Margin = new System.Windows.Forms.Padding(6, 5, 6, 5);
            this.textBox_AppLog.Multiline = true;
            this.textBox_AppLog.Name = "textBox_AppLog";
            this.textBox_AppLog.ReadOnly = true;
            this.textBox_AppLog.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.textBox_AppLog.Size = new System.Drawing.Size(581, 190);
            this.textBox_AppLog.TabIndex = 39;
            this.textBox_AppLog.TabStop = false;
            // 
            // label9
            // 
            this.label9.AutoSize = true;
            this.label9.Location = new System.Drawing.Point(14, 484);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(98, 21);
            this.label9.TabIndex = 40;
            this.label9.Text = "App Log:";
            // 
            // button_ClearAppLog
            // 
            this.button_ClearAppLog.Location = new System.Drawing.Point(612, 572);
            this.button_ClearAppLog.Name = "button_ClearAppLog";
            this.button_ClearAppLog.Size = new System.Drawing.Size(112, 50);
            this.button_ClearAppLog.TabIndex = 41;
            this.button_ClearAppLog.TabStop = false;
            this.button_ClearAppLog.Text = "Clear";
            this.button_ClearAppLog.UseVisualStyleBackColor = true;
            this.button_ClearAppLog.Click += new System.EventHandler(this.button_ClearAppLog_Click);
            // 
            // FormMain
            // 
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.None;
            this.ClientSize = new System.Drawing.Size(839, 852);
            this.Controls.Add(this.button_ClearAppLog);
            this.Controls.Add(this.label9);
            this.Controls.Add(this.textBox_AppLog);
            this.Controls.Add(this.button_ClearFwErr);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.textBox_FwErr);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.button_EraseEeprom);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.button_Can);
            this.Font = new System.Drawing.Font("ＭＳ ゴシック", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(128)));
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;
            this.Name = "FormMain";
            this.ShowIcon = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.FormMain_FormClosing);
            this.Load += new System.EventHandler(this.FormMain_Load);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.Label label_DeviceUid;
        private System.Windows.Forms.Button button_Connect;
        private System.Windows.Forms.ComboBox comboBox_Port;
        private System.Windows.Forms.Button button_Can;
        private System.Windows.Forms.Label label_FwName;
        private System.Windows.Forms.Label label_FwVer;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label_AppName;
        private System.Windows.Forms.Button button_EraseEeprom;
        private System.Windows.Forms.TextBox textBox_FwErr;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Timer timer;
        private System.Windows.Forms.Button button_ClearFwErr;
        private System.Windows.Forms.Label label_ConnectStatus;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label_AppVer;

        private System.Windows.Forms.TextBox textBox_AppLog;
        private System.Windows.Forms.Label label9;
        private System.Windows.Forms.Button button_ClearAppLog;
    }
}
