// Copyright © 2026 Shiomachi Software. All rights reserved.
using System;
using System.Collections.Generic;

namespace JigLib
{
    public abstract partial class JigCmd
    {
        /// <summary>
        /// 「FW情報取得」コマンドの要求を送信
        /// </summary>
        /// <param name="strMakerName">
        /// 取得したメーカー名(16文字)
        /// </param>
        /// <param name="strFwName">
        /// 取得したFW名(16文字)
        /// </param>
        /// <param name="strFwVer">
        /// 取得したFWバージョン(8文字の16進数文字列)
        /// </param>
        /// <param name="strDeviceUid">
        /// 取得したDevice Unique ID(32文字の16進数文字列)
        /// </param>
        /// <returns>
        /// エラーメッセージ。成功時はnullを返す。
        /// </returns>
        public string SendCmd_GetFwInfo(out string strMakerName, out string strFwName, out string strFwVer, out string strDeviceUid)
        {
            int offset = 0;
            byte[] aReqData = null;
            byte[] aResData = null;
            UInt32 fwVer;
            string strErrMsg;

            strMakerName = null;
            strFwName = null;
            strFwVer = null;
            strDeviceUid = null;

            strErrMsg = SendCmd(E_FRM_CMD.GET_FW_INFO, aReqData, out aResData);
            if (strErrMsg == null)
            {
                strMakerName = string.Empty;
                for (int i = 0; i < MAKER_NAME_SIZE; i++)
                {
                    if (aResData[i] != '\0')
                    {
                        strMakerName += (char)aResData[i];
                    }
                    else
                    {
                        break;
                    }
                }
                offset += MAKER_NAME_SIZE;

                strFwName = string.Empty;
                for (int i = 0; i < FW_NAME_SIZE; i++)
                {
                    if (aResData[offset + i] != '\0')
                    {
                        strFwName += (char)aResData[offset + i];
                    }
                    else
                    {
                        break;
                    }
                }
                offset += FW_NAME_SIZE;

                fwVer = BitConverter.ToUInt32(aResData, offset);
                offset += FW_VER_SIZE;
                strFwVer = fwVer.ToString("X8");

                strDeviceUid = string.Empty;
                for (int i = 0; i < DEVICE_UID_SIZE; i++)
                {
                    strDeviceUid += aResData[offset + i].ToString("X2");
                }
            }

            return strErrMsg;
        }

        /// <summary>
        /// 「FWエラー取得」コマンドの要求を送信
        /// </summary>
        /// <param name="lstErrMsg">
        /// 取得したエラーメッセージのリスト
        /// </param>
        /// <returns>
        /// エラーメッセージ。成功時はnullを返す。
        /// </returns>
        public string SendCmd_GetFwError(ref List<string> lstErrMsg)
        {
            byte[] aReqData = null;
            byte[] aResData = null;
            UInt32 errBits = 0;
            string strErrMsg;

            strErrMsg = SendCmd(E_FRM_CMD.GET_FW_ERR, aReqData, out aResData);
            if (strErrMsg == null)
            {
                errBits = BitConverter.ToUInt32(aResData, 0);
                if (lstErrMsg == null)
                {
                    lstErrMsg = new List<string>();
                }
                else
                {
                    lstErrMsg.Clear();
                }
                for (int i = 0; i < 32; i++) // 32ビットすべてをチェック
                {
                    if ((errBits & (1U << i)) != 0)
                    {
                        if (i < FW_ERR_MSG_ARY.Length)
                        {
                            lstErrMsg.Add(FW_ERR_MSG_ARY[i]);
                        }
                        else
                        {
                            lstErrMsg.Add("Undefined error");
                        }
                    }
                }
            }

            return strErrMsg;
        }

        /// <summary>
        /// 「FWエラークリア」コマンドの要求を送信
        /// </summary>
        /// <returns>
        /// エラーメッセージ。成功時はnullを返す。
        /// </returns>
        public string SendCmd_ClearFwError()
        {
            byte[] aReqData = null;
            byte[] aResData = null;
            string strErrMsg;

            strErrMsg = SendCmd(E_FRM_CMD.CLEAR_FW_ERR, aReqData, out aResData);

            return strErrMsg;
        }

        /// <summary>
        /// 「EEPROM消去」コマンドの要求を送信
        /// </summary>
        /// <returns>
        /// エラーメッセージ。成功時はnullを返す。
        /// </returns>
        public string SendCmd_EraseEeprom()
        {
            byte[] aReqData = null;
            byte[] aResData = null;
            string strErrMsg;

            strErrMsg = SendCmd(E_FRM_CMD.ERASE_EEPROM, aReqData, out aResData);

            return strErrMsg;
        }

        /// <summary>
        /// 「CAN設定変更」コマンドの要求を送信 (R4_BleCanBrg用: 9バイト)
        /// </summary>
        /// <param name="bitRate">CANビットレート</param>
        /// <param name="txId">送信CAN ID (TX ID)</param>
        /// <param name="idType">IDタイプ</param>
        /// <returns>エラーメッセージ。成功時はnullを返す。</returns>
        public string SendCmd_SetCanConfig(UInt32 bitRate, UInt32 txId, byte idType)
        {
            byte[] aReqData = new byte[CAN_CONFIG_PAYLOAD_SIZE];
            byte[] aResData = null;

            Array.Copy(BitConverter.GetBytes(bitRate), 0, aReqData, 0, 4);
            Array.Copy(BitConverter.GetBytes(txId), 0, aReqData, 4, 4);
            aReqData[8] = idType;

            return SendCmd(E_FRM_CMD.SET_CAN_CONFIG, aReqData, out aResData);
        }

        /// <summary>
        /// 「CAN設定変更」コマンドの要求を送信 (R4_CanEcho用: 13バイト)
        /// </summary>
        /// <param name="bitRate">CANビットレート</param>
        /// <param name="txId">送信CAN ID (TX ID)</param>
        /// <param name="idType">IDタイプ</param>
        /// <param name="rxId">受信CAN ID (RX ID)</param>
        /// <returns>エラーメッセージ。成功時はnullを返す。</returns>
        public string SendCmd_SetCanConfig(UInt32 bitRate, UInt32 txId, byte idType, UInt32 rxId)
        {
            byte[] aReqData = new byte[CAN_CONFIG_PAYLOAD_SIZE_ECHO];
            byte[] aResData = null;

            Array.Copy(BitConverter.GetBytes(bitRate), 0, aReqData, 0, 4);
            Array.Copy(BitConverter.GetBytes(txId), 0, aReqData, 4, 4);
            aReqData[8] = idType;
            Array.Copy(BitConverter.GetBytes(rxId), 0, aReqData, 9, 4);

            return SendCmd(E_FRM_CMD.SET_CAN_CONFIG, aReqData, out aResData);
        }

        /// <summary>
        /// 「CAN設定取得」コマンドの要求を送信 (R4_BleCanBrg用: 3引数)
        /// </summary>
        /// <param name="bitRate">取得したビットレート</param>
        /// <param name="txId">取得した送信CAN ID (TX ID)</param>
        /// <param name="idType">取得したIDタイプ</param>
        /// <returns>エラーメッセージ。成功時はnullを返す。</returns>
        public string SendCmd_GetCanConfig(out UInt32 bitRate, out UInt32 txId, out byte idType)
        {
            UInt32 rxId;
            return SendCmd_GetCanConfig(out bitRate, out txId, out idType, out rxId);
        }

        /// <summary>
        /// 「CAN設定取得」コマンドの要求を送信 (R4_CanEcho対応: 4引数)
        /// </summary>
        /// <param name="bitRate">取得したビットレート</param>
        /// <param name="txId">取得した送信CAN ID (TX ID)</param>
        /// <param name="idType">取得したIDタイプ</param>
        /// <param name="rxId">取得した受信CAN ID (RX ID: R4_CanEchoの場合のみ有効、BleCanBrg時は0)</param>
        /// <returns>エラーメッセージ。成功時はnullを返す。</returns>
        public string SendCmd_GetCanConfig(out UInt32 bitRate, out UInt32 txId, out byte idType, out UInt32 rxId)
        {
            byte[] aReqData = null;
            byte[] aResData = null;
            string strErrMsg;

            bitRate = 0;
            txId = 0;
            idType = 0;
            rxId = 0;

            strErrMsg = SendCmd(E_FRM_CMD.GET_CAN_CONFIG, aReqData, out aResData);
            if (strErrMsg == null && aResData != null && aResData.Length >= CAN_CONFIG_PAYLOAD_SIZE)
            {
                bitRate = BitConverter.ToUInt32(aResData, 0);
                txId = BitConverter.ToUInt32(aResData, 4);
                idType = aResData[8];

                if (aResData.Length >= CAN_CONFIG_PAYLOAD_SIZE_ECHO)
                {
                    rxId = BitConverter.ToUInt32(aResData, 9);
                }
            }

            return strErrMsg;
        }

        /// <summary>
        /// 要求フレームを送信
        /// </summary>
        private string SendCmd(E_FRM_CMD eCmd, byte[] aReqData, out byte[] aResData, int resTimeout = FRM_RES_TIMEOUT)
        {
            byte[] aReqFrm;            // 要求フレーム
            string strErrMsg = null;   // エラーメッセージ
            ST_FRM_REQ_FRAME stReqFrm; // 要求フレーム
            ST_FRM_RES_FRAME stResFrm; // 応答フレーム

            lock (_lockSend) // 送信～応答待ち中は、次の送信をしないようにするためのロック
            {
                aResData = null;

                // 応答フレーム受信キューを空にする
                PrpResEvent.Reset();
                while (true == PrpResFrmQue.TryTake(out stResFrm)) { }

                // [要求フレームを作成]
                stReqFrm.header = E_FRM_HEADER.REQ; // ヘッダ
                unchecked                           // オーバーフロー例外を防止
                {
                    stReqFrm.seqNo = _seqNo++;      // シーケンス番号
                }
                stReqFrm.cmd = eCmd;                // コマンド
                if (aReqData == null) // データ部が空の場合
                {
                    stReqFrm.dataSize = 0;  // データサイズ
                    stReqFrm.aData = null;  // データ
                }
                else // データ部が空ではない場合
                {
                    stReqFrm.dataSize = (UInt16)aReqData.Length; // データサイズ
                    stReqFrm.aData = aReqData;                   // データ
                }
                // チェックサム計算前の要求フレームのbyte型配列を取得
                stReqFrm.checksum = 0;
                aReqFrm = ConvertReqFrameStructToByteArray(stReqFrm);
                // チェックサムを計算 
                stReqFrm.checksum = CalcChecksum(aReqFrm, aReqFrm.Length - 2);

                // [要求フレームを送信]
                // 計算したチェックサムをbyte型配列に直接書き込む
                aReqFrm[aReqFrm.Length - 2] = (byte)(stReqFrm.checksum & 0xFF);
                aReqFrm[aReqFrm.Length - 1] = (byte)((stReqFrm.checksum >> 8) & 0xFF);

                // 要求フレームを送信    
                strErrMsg = Send(aReqFrm);
                if (strErrMsg != null)
                {
                    goto End;
                }

                // [応答フレーム受信イベント発生待ち]
                while (true)
                {
                    if (!PrpResEvent.WaitOne(resTimeout))
                    {
                        strErrMsg = "Response frame reception timeout.";
                        goto End;
                    }

                    // [応答フレーム受信キューから応答フレームを取り出す]
                    if (PrpResFrmQue.TryTake(out stResFrm))
                    {
                        if (stResFrm.seqNo != stReqFrm.seqNo || stResFrm.cmd != stReqFrm.cmd)
                        {
                            // 不一致の応答(以前のタイムアウトによるものなど)。イベントをリセットして再度待機。
                            PrpResEvent.Reset();
                            continue;
                        }

                        if (stResFrm.errCode == E_FRM_ERRCODE.SUCCESS)
                        {
                            aResData = new byte[stResFrm.aData.Length];
                            Array.Copy(stResFrm.aData, aResData, aResData.Length);
                        }
                        else
                        {
                            strErrMsg = ConvertErrCodeInResFrameToMsg(stResFrm.errCode);
                        }
                        goto End;
                    }
                    else
                    {
                        if (_isDisconnecting)
                        {
                            strErrMsg = STR_MSG_WAIT_RES_CANCEL;
                            goto End;
                        }
                        // イベントがセットされたがキューが空。リセットしてリトライ。
                        PrpResEvent.Reset();
                    }
                }
        End:

                return strErrMsg;
            }
        }

        /// <summary>
        /// 要求フレーム構造体をbyte型配列へ変換する
        /// </summary>
        private byte[] ConvertReqFrameStructToByteArray(ST_FRM_REQ_FRAME stReqFrm)
        {
            List<byte[]> lstByteAry = new List<byte[]>(); // byte型配列のリスト

            // 要求フレーム構造体の各フィールドをbyte型配列に変換してリストに追加
            lstByteAry.Add(new byte[1] { (byte)stReqFrm.header });
            lstByteAry.Add(BitConverter.GetBytes(stReqFrm.seqNo));
            lstByteAry.Add(BitConverter.GetBytes((UInt16)stReqFrm.cmd));
            lstByteAry.Add(BitConverter.GetBytes(stReqFrm.dataSize));
            lstByteAry.Add(stReqFrm.aData);
            lstByteAry.Add(BitConverter.GetBytes(stReqFrm.checksum));

            // リストを1つのbyte型配列に結合して返す
            return CombineByteArray(lstByteAry);
        }

        /// <summary>
        /// 引数のbyte型配列のリストを1つのbyte型配列に結合して返す
        /// </summary>
        private byte[] CombineByteArray(List<byte[]> lstByteAry)
        {
            // 返却するbyte型配列のサイズを求める
            int size = 0;
            foreach (byte[] ary in lstByteAry)
            {
                if (ary != null)
                {
                    size += ary.Length;
                }
            }

            // 引数のbyte型配列のリストを1つのbyte型配列に結合する
            int offset = 0;
            byte[] buf = new byte[size];
            foreach (byte[] ary in lstByteAry)
            {
                if (ary != null)
                {
                    Buffer.BlockCopy(ary, 0, buf, offset, ary.Length);
                    offset += ary.Length;
                }
            }

            return buf;
        }
    }
}
