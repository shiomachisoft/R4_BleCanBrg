// Copyright © 2026 Shiomachi Software. All rights reserved.
using System;
using System.Threading;

namespace JigLib
{
    public abstract partial class JigCmd
    {
        /// <summary>
        /// 受信データを取り出し・解析して受信フレーム(応答フレーム)を作成
        /// </summary>
        protected void Recv()
        {
            int dataSize = 0;   // 受信フレーム中のデータサイズ部
            byte data;          // シリアル受信データ(1byte)
            UInt16 expect;      // チェックサムの期待値

            DiscardRecvFrame(); // 初期化

            while (!_isDisconnecting)
            {
                bool isSleep = false;
                lock (_lockRecv) // 受信処理に関するリソースを排他する
                {
                    try
                    {
                        if (true == IsExistRecvData()) // 受信データが存在する場合
                        {
                            // 受信データを1byte取り出す
                            if (false == ReadByte(out data))
                            {
                                isSleep = true;
                                goto SkipParse;
                            }

                            // [受信データから受信フレームを作成する]

                            if (_recvSize == 0) // 受信データ = ヘッダ の場合(ヘッダはまだ格納していない場合)
                            {
                                switch (data) // ヘッダ
                                {
                                    case (byte)E_FRM_HEADER.RES: // 応答      
                                        // 受信フレームのバッファをゼロ充填
                                        Array.Clear(_bufRecvFrm, 0, _bufRecvFrm.Length);
                                        // ヘッダを格納
                                        _bufRecvFrm[_recvSize++] = data;
                                        // フレームエンドタイムアウト用タイマーを開始
                                        _timerFrameEnd.Change(FRM_END_TIMEOUT, Timeout.Infinite);
                                        break;
                                    default: // 不正なフレームヘッダ
                                        // 受信フレーム破棄
                                        DiscardRecvFrame();
                                        break;
                                }
                            }
                            else // ヘッダが格納済みの場合
                            {
                                if (_bufRecvFrm[0] == (byte)E_FRM_HEADER.RES) // ヘッダ = 応答 の場合
                                {
                                    if (_recvSize <= (FRM_RES_HEADER_SIZE - 1)) // 受信データがシーケンス番号/コマンド/エラーコード/データサイズの場合
                                    {
                                        // シーケンス番号/コマンド/エラーコード/データサイズを格納
                                        _bufRecvFrm[_recvSize++] = data;
                                        if (_recvSize == FRM_RES_HEADER_SIZE) // データサイズが格納済みの場合
                                        {
                                            // データサイズが最大値を超えてないかをチェック
                                            dataSize = _bufRecvFrm[8] << 8 | _bufRecvFrm[7];
                                            if (dataSize > FRM_DATA_MAX_SIZE) // データサイズが最大値を超えている場合
                                            {
                                                // 受信フレーム破棄
                                                DiscardRecvFrame();
                                            }
                                        }
                                    }
                                    else if (_recvSize <= (FRM_RES_HEADER_SIZE + dataSize + FRM_CHECKSUM_SIZE)) // 受信データ = データ/チェックサム の場合
                                    {
                                        // データ/チェックサムを格納
                                        _bufRecvFrm[_recvSize++] = data;
                                        if (_recvSize == (FRM_RES_HEADER_SIZE + dataSize + FRM_CHECKSUM_SIZE)) // チェックサムが格納済みの場合
                                        {
                                            // チェックサム検査
                                            expect = (UInt16)(((UInt16)_bufRecvFrm[_recvSize - 1]) << 8 | (UInt16)_bufRecvFrm[_recvSize - 2]);
                                            if (ChecksumTest(_bufRecvFrm, _recvSize - FRM_CHECKSUM_SIZE, expect)) // チェックサム検査がOKの場合
                                            {
                                                // 応答フレームをキューイング
                                                ST_FRM_RES_FRAME stResFrm = ConvertByteArrayToResponseFrame(_bufRecvFrm);
                                                if (PrpResFrmQue.TryAdd(stResFrm))
                                                {
                                                    // 応答フレーム受信をイベント通知
                                                    PrpResEvent.Set();
                                                }
                                            }
                                            // 受信フレーム解析完了
                                            DiscardRecvFrame();
                                        }
                                    }
                                }
                                else
                                {
                                    DiscardRecvFrame();
                                }
                            }
                        }
                        else // 受信データが存在しない場合
                        {
                            isSleep = true;
                        }
                    SkipParse:;
                    }
                    catch (Exception) // ポートのエラーが発生した場合
                    {
                        isSleep = true;
                    }
                }

                if (isSleep)
                {
                    Thread.Sleep(RECV_DELAY);
                }
            }
        }

        /// <summary>
        /// byte型配列を応答フレームに変換して返す
        /// </summary>
        private ST_FRM_RES_FRAME ConvertByteArrayToResponseFrame(byte[] buf)
        {
            ST_FRM_RES_FRAME stResFrm;

            stResFrm.header = (E_FRM_HEADER)buf[0];
            stResFrm.seqNo = (UInt16)(((UInt16)buf[2]) << 8 | (UInt16)buf[1]);
            stResFrm.cmd = (E_FRM_CMD)(((UInt16)buf[4]) << 8 | (UInt16)buf[3]);
            stResFrm.errCode = (E_FRM_ERRCODE)(((UInt16)buf[6]) << 8 | (UInt16)buf[5]);
            stResFrm.dataSize = (UInt16)(((UInt16)buf[8]) << 8 | (UInt16)buf[7]);
            if (stResFrm.dataSize > 0)
            {
                stResFrm.aData = new byte[stResFrm.dataSize];
                Buffer.BlockCopy(buf, 9, stResFrm.aData, 0, stResFrm.dataSize);
            }
            else
            {
                stResFrm.aData = Array.Empty<byte>();
            }
            int offset = 9 + stResFrm.dataSize;
            stResFrm.checksum = (UInt16)(((UInt16)buf[offset + 1]) << 8 | (UInt16)buf[offset]);

            return stResFrm;
        }

        /// <summary>
        /// チェックサム検査
        /// </summary>
        private bool ChecksumTest(byte[] buf, int size, UInt16 expected)
        {
            UInt16 checksum = 0;
            bool bRet = false;

            checksum = CalcChecksum(buf, size);
            if (checksum == expected)
            {
                bRet = true;
            }

            return bRet;
        }

        /// <summary>
        /// 受信フレーム(応答フレーム)のヘッダを受信後、フレームエンドタイムアウトの時間が経過してもそのフレームの末端を受信しなかった場合、そのフレームは破棄する
        /// </summary>
        private void FrameEndTimeoutCallback(Object state)
        {
            lock (_lockRecv) // 受信処理に関するリソースを排他する
            {
                // 受信フレーム破棄
                DiscardRecvFrame();
            }
        }

        /// <summary>
        /// 受信フレームの破棄(受信フレーム解析完了、または破棄処理)
        /// </summary>
        private void DiscardRecvFrame()
        {
            // 受信サイズ = 0
            _recvSize = 0;
            // フレームエンドタイムアウト用タイマーを停止
            _timerFrameEnd.Change(Timeout.Infinite, Timeout.Infinite);
        }

        /// <summary>
        /// 応答フレーム中のエラーコードをエラーメッセージに変換して返す
        /// </summary>
        private string ConvertErrCodeInResFrameToMsg(E_FRM_ERRCODE errCode)
        {
            string strErrMsg = null;

            // FWのソースのE_FRM_ERRに合わせる
            switch (errCode)
            {
                case E_FRM_ERRCODE.SUCCESS:
                    break;
                case E_FRM_ERRCODE.DATA_SIZE:
                    strErrMsg = "A failure response was received from the microcontroller. (The requested data size is invalid)";
                    break;
                case E_FRM_ERRCODE.PARAM:
                    strErrMsg = "A failure response was received from the microcontroller. (The argument in the request is invalid)";
                    break;
                default:
                    strErrMsg = "A failure response was received from the microcontroller. (Undefined error code)";
                    break;
            }

            return strErrMsg;
        }
    }
}
