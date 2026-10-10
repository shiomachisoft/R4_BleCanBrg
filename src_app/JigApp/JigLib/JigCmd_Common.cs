// Copyright © 2026 Shiomachi Software. All rights reserved.
using System;
using System.Collections.Concurrent;
using System.Threading;

namespace JigLib
{
    public abstract partial class JigCmd
    {
        /// <summary>
        /// 送信コマンドの応答待ちをキャンセルした時のメッセージ
        /// </summary>
        public const string STR_MSG_WAIT_RES_CANCEL = "Waiting for a response to the sent command has been canceled.";

        /// <summary>
        /// 要求フレーム最小ヘッダサイズ (header:1 + seqNo:2 + cmd:2 + dataSize:2)
        /// </summary>
        protected const int FRM_REQ_HEADER_SIZE = 7;
        /// <summary>
        /// 応答フレーム最小ヘッダサイズ (header:1 + seqNo:2 + cmd:2 + errCode:2 + dataSize:2)
        /// </summary>
        protected const int FRM_RES_HEADER_SIZE = 9;
        /// <summary>
        /// フレームチェックサムサイズ (UInt16: 2バイト)
        /// </summary>
        protected const int FRM_CHECKSUM_SIZE = 2;

        /// <summary>
        /// フレーム中のデータ部の最大サイズ
        /// </summary>
        /// <remarks>
        /// FW側のFRM_DATA_MAX_SIZEの値に合わせる
        /// </remarks>
        protected const int FRM_DATA_MAX_SIZE = 1024;
        /// <summary>
        /// 応答フレームの最大サイズ
        /// </summary>
        protected const int FRM_RES_SIZE = FRM_RES_HEADER_SIZE + FRM_DATA_MAX_SIZE + FRM_CHECKSUM_SIZE;

        /// <summary>
        /// CAN設定通信データ部サイズ (bitRate:4 + txId:4 + idType:1)
        /// </summary>
        public const int CAN_CONFIG_PAYLOAD_SIZE = 9;
        /// <summary>
        /// CanEcho用 CAN設定通信データ部サイズ (bitRate:4 + txId:4 + idType:1 + rxId:4)
        /// </summary>
        public const int CAN_CONFIG_PAYLOAD_SIZE_ECHO = 13;

        /// <summary>
        /// ファームウェア名: R4_BleCanBrg
        /// </summary>
        public const string FW_NAME_BLE_CAN_BRG = "R4_BleCanBrg";
        /// <summary>
        /// ファームウェア名: R4_CanEcho
        /// </summary>
        public const string FW_NAME_CAN_ECHO = "R4_CanEcho";

        /// <summary>
        /// メーカー名文字列サイズ (バイト)
        /// </summary>
        protected const int MAKER_NAME_SIZE = 16;
        /// <summary>
        /// ファームウェア名文字列サイズ (バイト)
        /// </summary>
        protected const int FW_NAME_SIZE = 16;
        /// <summary>
        /// ファームウェアバージョンサイズ (UInt32: 4バイト)
        /// </summary>
        protected const int FW_VER_SIZE = 4;
        /// <summary>
        /// マイコン固有識別子 (Device Unique ID) サイズ (バイト)
        /// </summary>
        protected const int DEVICE_UID_SIZE = 16;
        /// <summary>
        /// 受信タスクの終了待ちタイムアウト時間(ms)
        /// </summary>
        protected const int RECV_TASK_END_TIMEOUT = 20000;
        /// <summary>
        /// 応答フレーム受信タイムアウト(ms)
        /// </summary>
        protected const int FRM_RES_TIMEOUT = 10000;

        /// <summary>
        /// フレームエンドタイムアウト(ms) 
        /// </summary>
        /// <remarks>
        /// 受信フレーム(応答フレーム)のヘッダを受信後、フレームエンドタイムアウトの時間が経過してもそのフレームの末端を受信しなかった場合、そのフレームは破棄する
        /// </remarks>
        private const int FRM_END_TIMEOUT = 1500;
        /// <summary>
        /// Recv()のwhile文のディレイ(ms)
        /// </summary>
        private const int RECV_DELAY = 50;
        /// <summary>
        /// FWエラーメッセージ
        /// </summary>
        /// <remarks>
        /// FWのソースのCommon.hのdefineに合わせる
        /// </remarks>
        private readonly string[] FW_ERR_MSG_ARY =
        {
            // Bit 0: CAN transmission error (CMN_ERR_BIT_CAN_SEND_ERR)
            "CAN transmission error",
            // Bit 1: BLE transmission error (CMN_ERR_BIT_BLE_SEND_ERR)
            "BLE transmission error",
            // Bit 2: CAN initialization error (CMN_ERR_BIT_CAN_INIT_ERR)
            "CAN initialization error",
            // Bit 3: BLE to CAN buffer overflow (CMN_ERR_BIT_BUF_FULL_BLE_TO_CAN)
            "BLE to CAN buffer overflow",
            // Bit 4: CAN to BLE buffer overflow (CMN_ERR_BIT_BUF_FULL_CAN_TO_BLE)
            "CAN to BLE buffer overflow",
            // Bit 5: Microcontroller was reset by WDT timeout (CMN_ERR_BIT_WDT_RESET)
            "Microcontroller was reset by WDT timeout"
        };

        /// <summary>
        /// フレーム中のヘッダ部の定義
        /// </summary>
        protected enum E_FRM_HEADER : byte
        {
            /// <summary>
            /// 要求フレーム
            /// </summary>
            REQ = 0xA0,
            /// <summary>
            /// 応答フレーム
            /// </summary>
            RES = 0xA1
        }

        /// <summary>
        /// フレーム中のコマンド部の定義
        /// </summary>
        protected enum E_FRM_CMD : UInt16
        {
            /// <summary>
            /// FW情報取得
            /// </summary>
            GET_FW_INFO = 0x0001,
            /// <summary>
            /// FWエラー取得
            /// </summary>
            GET_FW_ERR = 0x0002,
            /// <summary>
            /// FWエラークリア
            /// </summary>
            CLEAR_FW_ERR = 0x0003,
            /// <summary>
            /// EEPROM消去
            /// </summary>
            ERASE_EEPROM = 0x0004,
            /// <summary>
            /// CAN設定変更
            /// </summary>
            SET_CAN_CONFIG = 0x0005,
            /// <summary>
            /// CAN設定取得
            /// </summary>
            GET_CAN_CONFIG = 0x0006
        }

        /// <summary>
        /// 応答フレーム中のエラーコード部の定義
        /// </summary>
        protected enum E_FRM_ERRCODE : UInt16
        {
            /// <summary>
            /// 成功
            /// </summary>
            SUCCESS = 0x0000,
            /// <summary>
            /// 要求中のデータ部のサイズが不正
            /// </summary>
            DATA_SIZE = 0x0001,
            /// <summary>
            /// 要求中の引数が不正
            /// </summary>
            PARAM = 0x0002
        }

        /// <summary>
        /// 要求フレーム構造体
        /// </summary>
        protected struct ST_FRM_REQ_FRAME
        {
            /// <summary>
            /// ヘッダ(1byte)
            /// </summary>
            public E_FRM_HEADER header;
            /// <summary>
            /// シーケンス番号(2byte)
            /// </summary>
            public UInt16 seqNo;
            /// <summary>
            /// コマンド(2byte)
            /// </summary>
            public E_FRM_CMD cmd;
            /// <summary>
            /// データサイズ(2byte)
            /// </summary>
            public UInt16 dataSize;
            /// <summary>
            /// データ
            /// </summary>
            public byte[] aData;
            /// <summary>
            /// チェックサム(2byte)
            /// </summary>
            public UInt16 checksum;
        }

        /// <summary>
        /// 応答フレーム構造体
        /// </summary>
        protected struct ST_FRM_RES_FRAME
        {
            /// <summary>
            /// ヘッダ(1byte)
            /// </summary>
            public E_FRM_HEADER header;
            /// <summary>
            /// シーケンス番号(2byte)
            /// </summary>
            public UInt16 seqNo;
            /// <summary>
            /// コマンド(2byte)
            /// </summary>
            public E_FRM_CMD cmd;
            /// <summary>
            /// エラーコード(2byte) 
            /// </summary>
            public E_FRM_ERRCODE errCode;
            /// <summary>
            /// データサイズ(2byte)
            /// </summary>
            public UInt16 dataSize;
            /// <summary>
            /// データ
            /// </summary>
            public byte[] aData;
            /// <summary>
            /// チェックサム(2byte)
            /// </summary>
            public UInt16 checksum;
        }

        /// <summary>
        /// 接続先名
        /// </summary>
        public string PrpConnectName { get; set; } = string.Empty;
        
        /// <summary>
        /// 接続済みか否か
        /// </summary>
        protected bool _isConnected = false;
        /// <summary>
        /// 応答フレーム受信イベント
        /// </summary>
        protected ManualResetEvent PrpResEvent { get; set; } = new ManualResetEvent(false);
        /// <summary>
        /// 応答フレームのキュー
        /// </summary>
        protected BlockingCollection<ST_FRM_RES_FRAME> PrpResFrmQue { get; set; } = new BlockingCollection<ST_FRM_RES_FRAME>(1);
        /// <summary>
        /// COMポートのアクセスを排他するためのロック用オブジェクト
        /// </summary>
        protected Object _lockPort = new Object();
        /// <summary>
        /// コマンド送信から応答受信待ちの間に、次の送信を抑止するためのロック用オブジェクト
        /// </summary>
        protected Object _lockSend = new Object();
        /// <summary>
        /// 受信処理に関するリソースを排他するロック用オブジェクト
        /// </summary>
        protected Object _lockRecv = new Object();
        /// <summary>
        /// 切断処理を実行中か否か
        /// </summary>
        protected bool _isDisconnecting = false;

        /// <summary>
        /// シーケンス番号
        /// </summary>
        private UInt16 _seqNo = 0;
        /// <summary>
        /// 応答フレームの受信サイズ
        /// </summary>
        private int _recvSize = 0;
        /// <summary>
        /// 応答フレームのバッファ
        /// </summary>
        private byte[] _bufRecvFrm = new byte[FRM_RES_SIZE];
        /// <summary>
        /// フレームエンドタイムアウト用タイマー
        /// </summary>
        private Timer _timerFrameEnd = null;

        /// <summary>
        /// コンストラクタ
        /// </summary>
        public JigCmd()
        {
            // フレームエンドタイムアウトのコールバックを登録
            _timerFrameEnd = new Timer(FrameEndTimeoutCallback, null, Timeout.Infinite, Timeout.Infinite);
        }

        /// <summary>
        /// チェックサムを計算
        /// </summary>
        private UInt16 CalcChecksum(byte[] buf, int size)
        {
            int i;
            UInt16 checksum = 0;

            unchecked // オーバーフロー例外を防止
            {
                for (i = 0; i < size; i++)
                {
                    checksum += buf[i];
                }
            }

            return checksum;
        }
    }
}
