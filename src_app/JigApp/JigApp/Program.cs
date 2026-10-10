// Copyright © 2026 Shiomachi Software. All rights reserved.
using System;
using System.Windows.Forms;
using JigLib;

namespace JigApp
{
    public static class Program
    {
        /// <summary>
        /// 治具コマンド(シリアル通信)のインスタンス
        /// </summary>
        public static JigSerial PrpJigSerial { get; set; } = new JigSerial();
        /// <summary>
        /// 治具コマンド(基底クラス)のインスタンス
        /// </summary>
        public static JigCmd PrpJigCmd { get; set; } = PrpJigSerial;
  
        /// <summary>
        /// アプリケーションのメイン エントリ ポイントです。
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new FormMain());
        }
    }
}
