# ファームウェアビルド・書き込み方法

本リポジトリに含まれる2つのファームウェアのビルドおよびマイコンボードへの書き込み手順を説明します。

1. **`R4_BleCanBrg`**: メイン機（Arduino UNO R4 WiFi 用 / BLE-CANブリッジ）
2. **`R4_CanEcho`**: 対向機（Arduino UNO R4 Minima 用 / CANエコーテスト用）

---

## 必要な環境

1. **Arduino IDE 2.3.8**
   - 公式サイトより Arduino IDE 2.3.8 をインストールします。

2. **ボードパッケージのインストール**
   - Arduino IDE の「ツール」⇒「ボード」⇒「ボードマネージャ」を開きます。
   - 検索欄に **`Arduino Renesas UNO R4 Boards`**（バージョン1.5.3） と入力し、インストールします。

3. **ライブラリのインストール (R4_BleCanBrg)**
   - Arduino IDE の「ツール」⇒「ライブラリを管理」を開きます。
   - 検索欄に **`ArduinoBLE`**（バージョン2.1.0） と入力し、インストールします。

---

## 1. R4_BleCanBrg のビルド・書き込み (Arduino UNO R4 WiFi)

### 1.1 スケッチを開く
- 本リポジトリの `src_fw/R4_BleCanBrg/R4_BleCanBrg.ino` を Arduino IDE で開きます。

### 1.2 ボードとポートの選択
1. Arduino UNO R4 WiFi をPCにUSB-Cケーブルで接続します。
2. 「ツール」⇒「ボード」⇒「Arduino Renesas UNO R4 Boards」⇒ **「Arduino UNO R4 WiFi」** を選択します。
3. 「ツール」⇒「ポート」⇒ 認識された Arduino UNO R4 WiFi の COMポートを選択します。

### 1.3 ビルドと書き込み
1. 「スケッチ」⇒「検証・コンパイル」を実行し、エラーなく完了することを確認します。
2. 「スケッチ」⇒「書き込む」をクリックして書き込みます。
3. 書き込みが完了するとマイコンが自動再起動し、12×8 LED Matrix 上でセンターバーが点滅を開始します（BLE未接続・アドバタイズ中）。

---

## 2. R4_CanEcho のビルド・書き込み (Arduino UNO R4 Minima)

### 2.1 スケッチを開く
- 本リポジトリの `src_fw/R4_CanEcho/R4_CanEcho.ino` を Arduino IDE で開きます。

### 2.2 ボードとポートの選択
1. Arduino UNO R4 Minima をPCにUSB-Cケーブルで接続します。
2. 「ツール」⇒「ボード」⇒「Arduino Renesas UNO R4 Boards」⇒ **「Arduino UNO R4 Minima」** を選択します。
3. 「ツール」⇒「ポート」⇒ 認識された Arduino UNO R4 Minima の COMポートを選択します。

### 2.3 ビルドと書き込み
1. 「スケッチ」⇒「検証・コンパイル」を実行し、エラーなく完了することを確認します。
2. 「スケッチ」⇒「書き込む」をクリックして書き込みます。
3. 書き込みが完了するとマイコンが自動再起動し、LEDが500ms間隔で点滅を開始します。
