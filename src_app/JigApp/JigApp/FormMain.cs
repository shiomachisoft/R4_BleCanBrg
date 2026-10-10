// Copyright © 2026 Shiomachi Software. All rights reserved.
using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO.Ports;
using System.Diagnostics;
using System.Reflection;
using JigLib;

namespace JigApp
{
    public partial class FormMain : Form
    {
        private const string STR_NOT_DISPLAYED = "---";
        private const string STR_BTN_CONNECT = "Connect";
        private const string STR_BTN_DISCONNECT = "Disconnect";
        private const string STR_LBL_CONNECT = "Connected";
        private const string STR_LBL_DISCONNECT = "Disconnected";

        /// <summary>
        /// タイトル状態: モニタ停止
        /// </summary>
        private const string STR_TITLE_MONITOR_STOPPED = "Monitor Stopped";
        /// <summary>
        /// タイトル状態: モニタ中
        /// </summary>
        private const string STR_TITLE_MONITORING = "Monitoring";

        /// <summary>
        /// マイコンが再起動するのを待つ時間(ms)
        /// </summary>
        private const int REBOOT_WAIT = 5000;
        /// <summary>
        /// 再接続リトライのタイムアウト時間(秒)
        /// </summary>
        private const int RECONNECT_TIME = 15;
        /// <summary>
        /// 再接続リトライ待ち時間(ms)
        /// </summary>
        private const int RECONNECT_RETRY_DELAY = 100;

        /// <summary>
        /// 自分のインスタンス
        /// </summary>
        public static FormMain Inst { get; set; } = null;

        /// <summary>
        /// 現在接続中のFW名
        /// </summary>
        public string CurrentFwName => label_FwName != null ? label_FwName.Text : string.Empty;

        /// <summary>
        /// アプリ名
        /// </summary>
        private string _strAppName = null;
        /// <summary>
        /// CANフォーム
        /// </summary>
        private FormCan _formCan = null;
        /// <summary>
        /// 子フォーム表示ボタンのリスト
        /// </summary>
        private List<Button> _lstButton = new List<Button>();
        /// <summary>
        /// FWエラーメッセージのリスト
        /// </summary>
        private List<string> _lstFwErrMsg = new List<string>();
        /// <summary>
        /// モニタ用タスク
        /// </summary>
        private Task<string> _tskMon = null;

        /// <summary>
        /// コンストラクタ
        /// </summary>
        public FormMain()
        {
            InitializeComponent();
            // 自分のインスタンスを保存
            Inst = this;
            // ボタンのリストを登録
            _lstButton.Add(button_Can);
            _lstButton.Add(button_EraseEeprom);
            _lstButton.Add(button_ClearFwErr);
        }

        /// <summary>
        /// フォームのロード時
        /// </summary>
        private void FormMain_Load(object sender, EventArgs e)
        {
            // アプリ名を表示
            _strAppName = Process.GetCurrentProcess().ProcessName;
            label_AppName.Text = _strAppName;
            // タイトルを表示
            this.Text = _strAppName + " - " + STR_TITLE_MONITOR_STOPPED;
            // アプリのバージョンを表示
            FileVersionInfo verInfo = FileVersionInfo.GetVersionInfo(Assembly.GetExecutingAssembly().Location);
            label_AppVer.Text = verInfo.FileVersion;
            // COMポート名の一覧をコンボボックスに追加
            AddSerialPortToList();
            // 接続状態ラベルの色を設定
            label_ConnectStatus.BackColor = UI.MonRed;
            // 子フォーム表示ボタンを無効に設定
            EnableFormButton(false);
        }

        /// <summary>
        /// COMポート名の一覧をコンボボックスに追加
        /// </summary>
        private void AddSerialPortToList()
        {
            string[] astrPortName;

            // COMポート名一覧を取得
            astrPortName = SerialPort.GetPortNames();
            Array.Sort(astrPortName); // ポート名の昇順にソート

            // ポート名一覧をコンボボックスに追加
            for (int i = 0; i < astrPortName.Length; i++)
            {
                if (!comboBox_Port.Items.Contains(astrPortName[i]))
                {
                    comboBox_Port.Items.Add(astrPortName[i]);
                }
            }

            if (comboBox_Port.Items.Count > 0) // コンボボックスのアイテム数が0より大きい場合
            {
                comboBox_Port.SelectedIndex = 0; // 先頭のアイテムを選択
            }
        }

        /// <summary>
        /// フォームを閉じる時
        /// </summary>
        private void FormMain_FormClosing(object sender, FormClosingEventArgs e)
        {
            // 破棄中にコールバックが呼ばれるのを防ぐためにタイマーを停止
            timer.Stop();
            // 切断する
            Program.PrpJigCmd.Disconnect();
        }

        /// <summary>
        /// タイマーコールバック
        /// </summary>
        private void timer_Tick(object sender, EventArgs e)
        {
            // 接続状態とFWエラーのモニタ
            Monitor();
        }

        /// <summary>
        /// 接続状態とFWエラーのモニタ
        /// </summary>
        private async void Monitor()
        {
            try
            {
                string strFwErrMsg;
                string strErrMsg;

                // [接続状態のモニタ]
                if ((!Program.PrpJigCmd.IsConnected()) && (label_ConnectStatus.Text == STR_LBL_CONNECT))
                {
                    // マイコンとの通信が切断された場合
                    AppendAppLogText(true, "Connection status is abnormal.");
                    // タイマーを止めて多重リトライを防ぐ
                    timer.Stop();
                    // 再接続する
                    await Reconnect();
                    return; // 以降の処理をスキップ
                }

                // [FWエラーのモニタ]
                if (true == Program.PrpJigCmd.IsConnected())
                {
                    if (_tskMon == null || _tskMon.IsCompleted)
                    {
                        List<string> lstTemp = null;
                        _tskMon = Task.Run(() =>
                        {
                            lstTemp = new List<string>();
                            // 「FWエラー取得」コマンドの要求を送信
                            return Program.PrpJigCmd.SendCmd_GetFwError(ref lstTemp);
                        });
                        strErrMsg = await _tskMon;

                        if (this.IsDisposed) return;

                        _lstFwErrMsg = lstTemp; // 代入はUIスレッドで行う

                        if (strErrMsg == null)
                        {
                            strFwErrMsg = string.Empty;
                            foreach (string strMsg in _lstFwErrMsg)
                            {
                                strFwErrMsg += (strMsg + "\r\n");
                            }
                            if (textBox_FwErr.Text != strFwErrMsg)
                            {
                                textBox_FwErr.Text = strFwErrMsg;
                            }
                            this.Text = _strAppName + " - " + Program.PrpJigCmd.PrpConnectName + " - " + STR_TITLE_MONITORING;
                        }
                        else
                        {
                            this.Text = _strAppName + " - " + STR_TITLE_MONITOR_STOPPED;
                            AppendAppLogText(true, strErrMsg);
                        }
                    }
                }
                else
                {
                    this.Text = _strAppName + " - " + STR_TITLE_MONITOR_STOPPED;
                }
            }
            catch (Exception ex)
            {
                if (!this.IsDisposed)
                {
                    AppendAppLogText(true, $"Monitor Error: {ex.Message}");
                }
            }
        }

        /// <summary>
        /// 「接続/切断」ボタンを押した時
        /// </summary>
        private async void button_Connect_Click(object sender, EventArgs e)
        {
            try
            {
                Program.PrpJigCmd = Program.PrpJigSerial;

                if (label_ConnectStatus.Text == STR_LBL_DISCONNECT) // 切断済みの場合
                {
                    // 接続する
                    await Connect();
                }
                else // 接続済みの場合
                {
                    // 切断する
                    Disconnect();
                }
            }
            catch (Exception ex)
            {
                if (!this.IsDisposed) UI.ShowErrMsg(this, $"Connect Error: {ex.Message}");
            }
        }

        /// <summary>
        /// 接続する
        /// </summary>
        private async Task Connect()
        {
            string strMakerName = STR_NOT_DISPLAYED; // メーカー名
            string strFwName = STR_NOT_DISPLAYED;    // FW名
            string strFwVer = STR_NOT_DISPLAYED;     // FWバージョン
            string strDeviceUid = STR_NOT_DISPLAYED; // Device Unique ID
            string strParam; // COMポート名
            string strErrMsg = null;

            if (comboBox_Port.Items.Count <= 0)
            {
                strErrMsg = "There are no COM ports recognized by Windows.\r\nPlease connect the microcontroller board to the PC via USB and then restart this application.";
                UI.ShowErrMsg(this, strErrMsg);
                return;
            }
            strParam = comboBox_Port.Text.Trim(); // COMポート名

            timer.Stop(); // 通信コマンドの競合を防ぐため、接続中はタイマーを停止する
            this.Enabled = false;
            AppendAppLogText(false, "Connecting...");
          
            try
            {
                DateTime dt_start = DateTime.Now;
                DateTime dt_end;
                TimeSpan ts;
                do
                {
                    // 接続する
                    // UIフリーズ防止のため別タスクで実行
                    strErrMsg = await Task.Run(() => Program.PrpJigCmd.Connect((Object)strParam));
                    if (strErrMsg == null)
                    {
                        break;
                    }
                    await Task.Delay(RECONNECT_RETRY_DELAY); // リトライ待ち
                    dt_end = DateTime.Now;
                    ts = dt_end - dt_start;
                } while (ts.TotalSeconds < RECONNECT_TIME);
                
                if (this.IsDisposed) return;
                    
                if (strErrMsg == null)
                {
                    // 「FW情報取得」コマンドの要求を送信
                    strErrMsg = await Task.Run(() =>
                    {
                        return Program.PrpJigCmd.SendCmd_GetFwInfo(out strMakerName, out strFwName, out strFwVer, out strDeviceUid);
                    });
                    
                    if (this.IsDisposed) return;
                    
                    if (strErrMsg != null)
                    {
                        strErrMsg = "Failed to retrieve firmware information after connecting.\n\n" + strErrMsg;
                    }
                }
                
                if (strErrMsg == null) // 接続およびコマンドが成功した場合
                {
                    // [表示を更新]
                    // COMポート名一覧のコンボボックスを無効に設定
                    comboBox_Port.Enabled = false;
                    // 接続状態
                    AppendAppLogText(false, "Connected");
                    label_ConnectStatus.Text = STR_LBL_CONNECT;
                    label_ConnectStatus.BackColor = UI.MonGreen;
                    // ボタンの表示を「切断する」に変更
                    button_Connect.Text = STR_BTN_DISCONNECT;
                    // FW名
                    label_FwName.Text = strFwName;
                    // FWバージョン
                    label_FwVer.Text = strFwVer;
                    // Device Unique ID
                    label_DeviceUid.Text = strDeviceUid;
                    // 子フォーム表示ボタンを有効に設定
                    EnableFormButton(true);
                }
                else // 接続が失敗 または コマンドが失敗した場合
                {
                    UI.ShowErrMsg(this, strErrMsg);
                    // 切断する
                    Program.PrpJigCmd.Disconnect();
                }
            }
            finally
            {
                if (!this.IsDisposed)
                {
                    timer.Start(); // 接続処理が完了したらタイマーを再開する
                    this.Enabled = true;
                }
            }
        }

        /// <summary>
        /// 切断する
        /// </summary>
        private void Disconnect()
        {
            string strFwName = STR_NOT_DISPLAYED;  // FW名
            string strFwVer = STR_NOT_DISPLAYED;   // FWバージョン
            string strDeviceUid = STR_NOT_DISPLAYED; // Device Unique ID
            
            // 切断する
            Program.PrpJigCmd.Disconnect();
            // [表示を更新]
            // COMポート名一覧のコンボボックスを有効に設定
            comboBox_Port.Enabled = true;
            // 接続状態
            AppendAppLogText(false, "Disconnected");
            label_ConnectStatus.Text = STR_LBL_DISCONNECT;
            label_ConnectStatus.BackColor = UI.MonRed;
            // ボタンの表示を「接続する」に変更
            button_Connect.Text = STR_BTN_CONNECT;
            // FW名
            label_FwName.Text = strFwName;
            // FWバージョン
            label_FwVer.Text = strFwVer;
            // Device Unique ID
            label_DeviceUid.Text = strDeviceUid;
            // 子フォーム表示ボタンを無効に設定
            EnableFormButton(false);     
        }

        /// <summary>
        /// 再接続する
        /// </summary>
        /// <remarks>
        /// マイコンが再起動されるようなコマンドが成功した後に本関数を使用する
        /// </remarks>
        public async Task Reconnect()
        {
            // 他のフォームから本関数が呼ばれた時に、メインフォームが既に破棄されている場合は何もしない
            if (true == this.IsDisposed)
            {
                return;
            }

            try
            {
                // リトライ中にMonitorが走るのを防ぐためタイマー停止
                timer.Stop(); 

                // 切断する
                Disconnect();
                this.Enabled = false;
                AppendAppLogText(false, "Reconnecting...");
                
                // マイコンが再起動するのを待つ
                await Task.Delay(REBOOT_WAIT);
                
                if (this.IsDisposed) return;
                
                // 接続する
                await Connect();
            }
            catch (Exception ex)
            {
                if (!this.IsDisposed)
                {
                    AppendAppLogText(true, $"Reconnect Error: {ex.Message}");
                }
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
        /// 子フォーム表示ボタンの有効/無効を設定
        /// </summary>
        private void EnableFormButton(bool bEnable)
        {
            foreach (Button btn in _lstButton)
            {
                btn.Enabled = bEnable;
            }
        }

        /// <summary>
        /// 「CAN」ボタンを押した時
        /// </summary>
        private void button_Can_Click(object sender, EventArgs e)
        {
            // CANフォームを表示
            ShowChildForm((Button)sender);
        }

        /// <summary>
        /// 子フォーム表示ボタンに応じた子フォームを表示
        /// </summary>
        private void ShowChildForm(Button button)
        {
            Form frm = null;

            if (button == button_Can) // CAN
            {
                frm = _formCan;
                if (frm == null || frm.IsDisposed)
                {
                    frm = _formCan = new FormCan();
                    frm.Show();
                }
            }

            if (frm != null)
            {
                // フォームを適切に前面に持ってくる
                frm.Activate();
                
                // 子フォームが最小化されている時、元の状態に戻す
                if (frm.WindowState == FormWindowState.Minimized)
                {
                    frm.WindowState = FormWindowState.Normal;
                }
            }
        }

        /// <summary>
        /// 「設定データ消去」ボタンを押した時
        /// </summary>
        private async void button_EraseEeprom_Click(object sender, EventArgs e)
        {
            try
            {
                string strErrMsg;

                // 確認メッセージを表示
                if (DialogResult.No == UI.ShowYesNoMsg(this, "Do you want to erase the setting data in the EEPROM?\n\n(The microcontroller will be reset.)"))
                {
                    return;
                }

                // 「EEPROM消去」コマンドの要求を送信
                this.Enabled = false;
                strErrMsg = await Task.Run(() =>
                {
                    return Program.PrpJigCmd.SendCmd_EraseEeprom();
                });
                if (this.IsDisposed) return;
                
                if (strErrMsg == null)
                {
                    // 再接続する
                    await Reconnect();
                }
                else
                {
                    UI.ShowErrMsg(this, strErrMsg);
                }
            }
            catch (Exception ex)
            {
                if (!this.IsDisposed) UI.ShowErrMsg(this, $"Erase EEPROM Error: {ex.Message}");
            }
            finally
            {
                if (!this.IsDisposed) this.Enabled = true;
            }
        }

        /// <summary>
        /// 「Appログ」テキストボックスにログを追加する
        /// </summary>
        public void AppendAppLogText(bool bError, string strMsg)
        {
            // 他のフォームから本関数が呼ばれた時に、メインフォームが既に破棄されている場合は何もしない
            if (true == this.IsDisposed)
            {
                return;
            }

            // スレッドセーフな呼び出しにする(クロススレッド例外回避)
            if (this.InvokeRequired)
            {
                this.Invoke(new Action(() => AppendAppLogText(bError, strMsg)));
                return;
            }

            string strLog;

            // 送信コマンドの応答待ちをキャンセルした時のメッセージを表示しないようにする
            if (strMsg == JigCmd.STR_MSG_WAIT_RES_CANCEL)
            {
                // 無処理
            }
            else
            {
                if (bError)
                {
                    strMsg = "Err!!! " + strMsg;
                }
                strLog = "[" + DateTime.Now.ToString("HH:mm:ss") + "] " + strMsg + "\r\n";
                textBox_AppLog.AppendText(strLog);
            }
        }

        /// <summary>
        /// 「Appログクリア」ボタンを押した時
        /// </summary>
        private void button_ClearAppLog_Click(object sender, EventArgs e)
        {
            textBox_AppLog.Text = string.Empty;
        }

        /// <summary>
        /// 「FWエラークリア」ボタンを押した時
        /// </summary>
        private async void button_ClearFwErr_Click(object sender, EventArgs e)
        {
            try
            {
                string strErrMsg;

                this.Enabled = false;
                strErrMsg = await Task.Run(() =>
                {
                    // 「FWエラークリア」コマンドの要求を送信
                    return Program.PrpJigCmd.SendCmd_ClearFwError();
                });
                if (this.IsDisposed) return;
                
                if (strErrMsg == null)
                {
                    // 無処理
                }
                else
                {
                    UI.ShowErrMsg(this, strErrMsg);
                }
            }
            catch (Exception ex)
            {
                if (!this.IsDisposed) UI.ShowErrMsg(this, $"Clear FW Error: {ex.Message}");
            }
            finally
            {
                if (!this.IsDisposed) this.Enabled = true;
            }
        }
    }
}
