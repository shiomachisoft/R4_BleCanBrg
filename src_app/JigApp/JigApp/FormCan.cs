// Copyright © 2026 Shiomachi Software. All rights reserved.
using System;
using System.Globalization;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace JigApp
{
    public partial class FormCan : Form
    {
        /// <summary>
        /// CANボーレート: 125 kbps
        /// </summary>
        private const UInt32 CAN_BITRATE_125K = 125000;
        /// <summary>
        /// CANボーレート: 250 kbps
        /// </summary>
        private const UInt32 CAN_BITRATE_250K = 250000;
        /// <summary>
        /// CANボーレート: 500 kbps (デフォルト)
        /// </summary>
        private const UInt32 CAN_BITRATE_500K = 500000;
        /// <summary>
        /// CANボーレート: 1000 kbps (1 Mbps)
        /// </summary>
        private const UInt32 CAN_BITRATE_1000K = 1000000;

        /// <summary>
        /// デフォルト送信CAN ID (TX ID)
        /// </summary>
        private const UInt32 CAN_DEFAULT_TX_ID = 0x100;

        /// <summary>
        /// CAN IDタイプ: Standard
        /// </summary>
        private const byte CAN_ID_TYPE_STANDARD = 0;
        /// <summary>
        /// CAN IDタイプ: Extended
        /// </summary>
        private const byte CAN_ID_TYPE_EXTENDED = 1;

        /// <summary>
        /// Standard CAN ID 最大値 (11-bit: 0x7FF)
        /// </summary>
        private const UInt32 CAN_ID_STANDARD_MAX = 0x7FF;
        /// <summary>
        /// Extended CAN ID 最大値 (29-bit: 0x1FFFFFFF)
        /// </summary>
        private const UInt32 CAN_ID_EXTENDED_MAX = 0x1FFFFFFF;

        /// <summary>
        /// ボーレートコンボボックス選択インデックス: 125 kbps
        /// </summary>
        private const int IDX_BAUDRATE_125K = 0;
        /// <summary>
        /// ボーレートコンボボックス選択インデックス: 250 kbps
        /// </summary>
        private const int IDX_BAUDRATE_250K = 1;
        /// <summary>
        /// ボーレートコンボボックス選択インデックス: 500 kbps
        /// </summary>
        private const int IDX_BAUDRATE_500K = 2;
        /// <summary>
        /// ボーレートコンボボックス選択インデックス: 1000 kbps
        /// </summary>
        private const int IDX_BAUDRATE_1000K = 3;

        private string _strTitle;

        /// <summary>
        /// コンストラクタ
        /// </summary>
        public FormCan()
        {
            InitializeComponent();
            _strTitle = this.Text;
        }

        /// <summary>
        /// フォームのロード時
        /// </summary>
        private void FormCan_Load(object sender, EventArgs e)
        {
            bool isEcho = FormMain.Inst.CurrentFwName == JigLib.JigCmd.FW_NAME_CAN_ECHO;
            if (!isEcho)
            {
                // R4_BleCanBrg の場合: RX CAN ID は非表示にし、元のコンパクトなレイアウトへ戻す
                label_RxId.Visible = false;
                textBox_RxId.Visible = false;
            }
            else
            {
                // R4_CanEcho の場合: RX CAN ID を表示
                label_RxId.Visible = true;
                textBox_RxId.Visible = true;
            }

            comboBox_Baudrate.Items.Add("125 kbps");
            comboBox_Baudrate.Items.Add("250 kbps");
            comboBox_Baudrate.Items.Add("500 kbps");
            comboBox_Baudrate.Items.Add("1000 kbps");
            comboBox_Baudrate.SelectedIndex = IDX_BAUDRATE_500K; // Default 500 kbps

            GetConfig();
        }

        /// <summary>
        /// CAN設定を取得して画面に反映
        /// </summary>
        private async void GetConfig()
        {
            try
            {
                UInt32 bitRate = 0;
                UInt32 txId = CAN_DEFAULT_TX_ID;
                byte idType = CAN_ID_TYPE_STANDARD;
                UInt32 rxId = 0;
                string strErrMsg = null;

                this.Enabled = false;
                strErrMsg = await Task.Run(() =>
                {
                    return Program.PrpJigCmd.SendCmd_GetCanConfig(out bitRate, out txId, out idType, out rxId);
                });

                if (strErrMsg == null)
                {
                    switch (bitRate)
                    {
                        case CAN_BITRATE_125K:  comboBox_Baudrate.SelectedIndex = IDX_BAUDRATE_125K; break;
                        case CAN_BITRATE_250K:  comboBox_Baudrate.SelectedIndex = IDX_BAUDRATE_250K; break;
                        case CAN_BITRATE_500K:  comboBox_Baudrate.SelectedIndex = IDX_BAUDRATE_500K; break;
                        case CAN_BITRATE_1000K: comboBox_Baudrate.SelectedIndex = IDX_BAUDRATE_1000K; break;
                        default: comboBox_Baudrate.SelectedIndex = IDX_BAUDRATE_500K; break;
                    }

                    textBox_TxId.Text = txId.ToString("X");
                    textBox_RxId.Text = rxId.ToString("X");
                    if (idType == CAN_ID_TYPE_EXTENDED)
                    {
                        radioButton_Extended.Checked = true;
                    }
                    else
                    {
                        radioButton_Standard.Checked = true;
                    }
                }
                else
                {
                    UI.ShowErrMsg(this, strErrMsg);
                }
            }
            catch (Exception ex)
            {
                UI.ShowErrMsg(this, ex.Message);
            }
            finally
            {
                this.Enabled = true;
            }
        }

        /// <summary>
        /// 「設定変更」ボタンを押した時
        /// </summary>
        private async void button_SetConfig_Click(object sender, EventArgs e)
        {
            try
            {
                // Display confirmation message / 確認メッセージを表示
                if (DialogResult.No == UI.ShowYesNoMsg(this, "Do you want to save settings to EEPROM?\n\n(The microcontroller will be reset.)"))
                {
                    return;
                }

                UInt32 bitRate = CAN_BITRATE_500K;
                switch (comboBox_Baudrate.SelectedIndex)
                {
                    case IDX_BAUDRATE_125K:  bitRate = CAN_BITRATE_125K; break;
                    case IDX_BAUDRATE_250K:  bitRate = CAN_BITRATE_250K; break;
                    case IDX_BAUDRATE_500K:  bitRate = CAN_BITRATE_500K; break;
                    case IDX_BAUDRATE_1000K: bitRate = CAN_BITRATE_1000K; break;
                }

                string strTxId = textBox_TxId.Text.Trim();
                if (strTxId.StartsWith("0x", StringComparison.OrdinalIgnoreCase))
                {
                    strTxId = strTxId.Substring(2);
                }

                if (!UInt32.TryParse(strTxId, NumberStyles.HexNumber, CultureInfo.InvariantCulture, out UInt32 txId))
                {
                    UI.ShowErrMsg(this, "Please enter a valid hexadecimal TX CAN ID.");
                    return;
                }

                byte idType = radioButton_Extended.Checked ? CAN_ID_TYPE_EXTENDED : CAN_ID_TYPE_STANDARD;
                if (idType == CAN_ID_TYPE_STANDARD && txId > CAN_ID_STANDARD_MAX)
                {
                    UI.ShowErrMsg(this, "Standard TX CAN ID must be between 0x000 and 0x7FF.");
                    return;
                }
                else if (idType == CAN_ID_TYPE_EXTENDED && txId > CAN_ID_EXTENDED_MAX)
                {
                    UI.ShowErrMsg(this, "Extended TX CAN ID must be between 0x00000000 and 0x1FFFFFFF.");
                    return;
                }

                bool isEcho = FormMain.Inst.CurrentFwName == JigLib.JigCmd.FW_NAME_CAN_ECHO;
                UInt32 rxId = 0;
                if (isEcho)
                {
                    string strRxId = textBox_RxId.Text.Trim();
                    if (strRxId.StartsWith("0x", StringComparison.OrdinalIgnoreCase))
                    {
                        strRxId = strRxId.Substring(2);
                    }

                    if (!UInt32.TryParse(strRxId, NumberStyles.HexNumber, CultureInfo.InvariantCulture, out rxId))
                    {
                        UI.ShowErrMsg(this, "Please enter a valid hexadecimal RX CAN ID.");
                        return;
                    }

                    if (idType == CAN_ID_TYPE_STANDARD && rxId > CAN_ID_STANDARD_MAX)
                    {
                        UI.ShowErrMsg(this, "Standard RX CAN ID must be between 0x000 and 0x7FF.");
                        return;
                    }
                    else if (idType == CAN_ID_TYPE_EXTENDED && rxId > CAN_ID_EXTENDED_MAX)
                    {
                        UI.ShowErrMsg(this, "Extended RX CAN ID must be between 0x00000000 and 0x1FFFFFFF.");
                        return;
                    }

                    if (txId == rxId)
                    {
                        UI.ShowErrMsg(this, "RX CAN ID and TX CAN ID must be different to prevent echo loops.");
                        return;
                    }
                }

                this.Enabled = false;
                string strErrMsg = await Task.Run(() =>
                {
                    if (isEcho)
                    {
                        return Program.PrpJigCmd.SendCmd_SetCanConfig(bitRate, txId, idType, rxId);
                    }
                    else
                    {
                        return Program.PrpJigCmd.SendCmd_SetCanConfig(bitRate, txId, idType);
                    }
                });
                if (this.IsDisposed) return;

                if (strErrMsg == null)
                {
                    // Reconnect / 再接続する
                    await FormMain.Inst.Reconnect();
                    this.Close();
                }
                else
                {
                    UI.ShowErrMsg(this, strErrMsg);
                }
            }
            catch (Exception ex)
            {
                UI.ShowErrMsg(this, ex.Message);
            }
            finally
            {
                if (!this.IsDisposed)
                {
                    this.Enabled = true;
                }
            }
        }

        /// <summary>
        /// タイマーコールバック
        /// </summary>
        private void timer_Tick(object sender, EventArgs e)
        {
            if (true == Program.PrpJigCmd.IsConnected())
            {
                this.Text = _strTitle + " - " + Program.PrpJigCmd.PrpConnectName + " - " + "Connected";
            }
            else
            {
                this.Text = _strTitle + " - " + "Disconnected";
            }
        }
    }
}
