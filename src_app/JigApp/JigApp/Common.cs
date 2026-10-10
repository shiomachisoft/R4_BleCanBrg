// Copyright © 2026 Shiomachi Software. All rights reserved.
using System;
using System.Drawing;
using System.Windows.Forms;

namespace JigApp
{
    /// <summary>
    /// UI操作のクラス
    /// </summary>
    public static class UI
    {
        /// <summary>
        /// モニタ用の'赤'を返す
        /// </summary>
        public static Color MonRed { get; set; } = Color.FromArgb(255, 150, 150);

        /// <summary>
        /// モニタ用の'緑'を返す
        /// </summary>
        public static Color MonGreen { get; set; } = Color.FromArgb(150, 255, 150);

        /// <summary>
        /// 通知のメッセージボックスを表示
        /// </summary>
        public static void ShowInfoMsg(Form frm, string strMsg)
        {
            MessageBox.Show(frm, strMsg, "Information", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }

        /// <summary>
        /// Yes/No確認のメッセージボックスを表示
        /// </summary>
        public static DialogResult ShowYesNoMsg(Form frm, string strMsg)
        {
            return MessageBox.Show(frm, strMsg, "Question", MessageBoxButtons.YesNo, MessageBoxIcon.Question);
        }

        /// <summary>
        /// エラーのメッセージボックスを表示
        /// </summary>
        public static void ShowErrMsg(Form frm, string strMsg)
        {
            FormMain.Inst.AppendAppLogText(true, strMsg);
            MessageBox.Show(frm, strMsg, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
        }
    }
}
