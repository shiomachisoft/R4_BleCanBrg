namespace JigApp
{
    partial class FormCan
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.label_ExtendedRange = new System.Windows.Forms.Label();
            this.label_StandardRange = new System.Windows.Forms.Label();
            this.radioButton_Extended = new System.Windows.Forms.RadioButton();
            this.radioButton_Standard = new System.Windows.Forms.RadioButton();
            this.label3 = new System.Windows.Forms.Label();
            this.textBox_RxId = new System.Windows.Forms.TextBox();
            this.label_RxId = new System.Windows.Forms.Label();
            this.textBox_TxId = new System.Windows.Forms.TextBox();
            this.label_TxId = new System.Windows.Forms.Label();
            this.button_SetConfig = new System.Windows.Forms.Button();
            this.comboBox_Baudrate = new System.Windows.Forms.ComboBox();
            this.label1 = new System.Windows.Forms.Label();
            this.timer = new System.Windows.Forms.Timer(this.components);
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.label_ExtendedRange);
            this.groupBox1.Controls.Add(this.label_StandardRange);
            this.groupBox1.Controls.Add(this.radioButton_Extended);
            this.groupBox1.Controls.Add(this.radioButton_Standard);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.textBox_RxId);
            this.groupBox1.Controls.Add(this.label_RxId);
            this.groupBox1.Controls.Add(this.textBox_TxId);
            this.groupBox1.Controls.Add(this.label_TxId);
            this.groupBox1.Controls.Add(this.button_SetConfig);
            this.groupBox1.Controls.Add(this.comboBox_Baudrate);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Location = new System.Drawing.Point(15, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(805, 342);
            this.groupBox1.TabIndex = 24;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "CAN Settings";
            // 
            // label_ExtendedRange
            // 
            this.label_ExtendedRange.AutoSize = true;
            this.label_ExtendedRange.Location = new System.Drawing.Point(434, 191);
            this.label_ExtendedRange.Name = "label_ExtendedRange";
            this.label_ExtendedRange.Size = new System.Drawing.Size(362, 21);
            this.label_ExtendedRange.TabIndex = 28;
            this.label_ExtendedRange.Text = "(Range: 0x00000000 - 0x1FFFFFFF)";
            // 
            // label_StandardRange
            // 
            this.label_StandardRange.AutoSize = true;
            this.label_StandardRange.Location = new System.Drawing.Point(434, 155);
            this.label_StandardRange.Name = "label_StandardRange";
            this.label_StandardRange.Size = new System.Drawing.Size(252, 21);
            this.label_StandardRange.TabIndex = 27;
            this.label_StandardRange.Text = "(Range: 0x000 - 0x7FF)";
            // 
            // radioButton_Extended
            // 
            this.radioButton_Extended.AutoSize = true;
            this.radioButton_Extended.Location = new System.Drawing.Point(209, 189);
            this.radioButton_Extended.Name = "radioButton_Extended";
            this.radioButton_Extended.Size = new System.Drawing.Size(222, 25);
            this.radioButton_Extended.TabIndex = 26;
            this.radioButton_Extended.Text = "Extended (29-bit)";
            this.radioButton_Extended.UseVisualStyleBackColor = true;
            // 
            // radioButton_Standard
            // 
            this.radioButton_Standard.AutoSize = true;
            this.radioButton_Standard.Checked = true;
            this.radioButton_Standard.Location = new System.Drawing.Point(209, 153);
            this.radioButton_Standard.Name = "radioButton_Standard";
            this.radioButton_Standard.Size = new System.Drawing.Size(222, 25);
            this.radioButton_Standard.TabIndex = 25;
            this.radioButton_Standard.TabStop = true;
            this.radioButton_Standard.Text = "Standard (11-bit)";
            this.radioButton_Standard.UseVisualStyleBackColor = true;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(15, 155);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(142, 21);
            this.label3.TabIndex = 24;
            this.label3.Text = "CAN ID Type:";
            // 
            // textBox_RxId
            // 
            this.textBox_RxId.Location = new System.Drawing.Point(209, 112);
            this.textBox_RxId.Name = "textBox_RxId";
            this.textBox_RxId.Size = new System.Drawing.Size(430, 28);
            this.textBox_RxId.TabIndex = 30;
            this.textBox_RxId.TabStop = false;
            this.textBox_RxId.Text = "100";
            // 
            // label_RxId
            // 
            this.label_RxId.AutoSize = true;
            this.label_RxId.Location = new System.Drawing.Point(15, 115);
            this.label_RxId.Name = "label_RxId";
            this.label_RxId.Size = new System.Drawing.Size(186, 21);
            this.label_RxId.TabIndex = 29;
            this.label_RxId.Text = "RX CAN ID (Hex):";
            // 
            // textBox_TxId
            // 
            this.textBox_TxId.Location = new System.Drawing.Point(209, 72);
            this.textBox_TxId.Name = "textBox_TxId";
            this.textBox_TxId.Size = new System.Drawing.Size(430, 28);
            this.textBox_TxId.TabIndex = 23;
            this.textBox_TxId.TabStop = false;
            this.textBox_TxId.Text = "100";
            // 
            // label_TxId
            // 
            this.label_TxId.AutoSize = true;
            this.label_TxId.Location = new System.Drawing.Point(15, 75);
            this.label_TxId.Name = "label_TxId";
            this.label_TxId.Size = new System.Drawing.Size(186, 21);
            this.label_TxId.TabIndex = 22;
            this.label_TxId.Text = "TX CAN ID (Hex):";
            // 
            // button_SetConfig
            // 
            this.button_SetConfig.Location = new System.Drawing.Point(270, 260);
            this.button_SetConfig.Name = "button_SetConfig";
            this.button_SetConfig.Size = new System.Drawing.Size(215, 56);
            this.button_SetConfig.TabIndex = 21;
            this.button_SetConfig.TabStop = false;
            this.button_SetConfig.Text = "Set Config";
            this.button_SetConfig.UseVisualStyleBackColor = true;
            this.button_SetConfig.Click += new System.EventHandler(this.button_SetConfig_Click);
            // 
            // comboBox_Baudrate
            // 
            this.comboBox_Baudrate.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboBox_Baudrate.FormattingEnabled = true;
            this.comboBox_Baudrate.Location = new System.Drawing.Point(209, 32);
            this.comboBox_Baudrate.Name = "comboBox_Baudrate";
            this.comboBox_Baudrate.Size = new System.Drawing.Size(430, 29);
            this.comboBox_Baudrate.TabIndex = 19;
            this.comboBox_Baudrate.TabStop = false;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(15, 35);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(120, 21);
            this.label1.TabIndex = 18;
            this.label1.Text = "Baud Rate:";
            // 
            // timer
            // 
            this.timer.Enabled = true;
            this.timer.Interval = 1000;
            this.timer.Tick += new System.EventHandler(this.timer_Tick);
            // 
            // FormCan
            // 
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.None;
            this.ClientSize = new System.Drawing.Size(837, 369);
            this.Controls.Add(this.groupBox1);
            this.Font = new System.Drawing.Font("ＭＳ ゴシック", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(128)));
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.MaximizeBox = false;
            this.Name = "FormCan";
            this.ShowIcon = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "CAN";
            this.Load += new System.EventHandler(this.FormCan_Load);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.Button button_SetConfig;
        private System.Windows.Forms.ComboBox comboBox_Baudrate;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox textBox_TxId;
        private System.Windows.Forms.Label label_TxId;
        private System.Windows.Forms.TextBox textBox_RxId;
        private System.Windows.Forms.Label label_RxId;
        private System.Windows.Forms.RadioButton radioButton_Extended;
        private System.Windows.Forms.RadioButton radioButton_Standard;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label_StandardRange;
        private System.Windows.Forms.Label label_ExtendedRange;
        private System.Windows.Forms.Timer timer;
    }
}
