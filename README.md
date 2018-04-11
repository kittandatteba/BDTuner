# BDTuner

## これは何？
ブルドッグ・チューナーことAKB-PCR20X用を各種アプリケーションから使用するためのライブラリ（DLL）です。  
PT3-SDK（SDK_EARTHSOFT_PT3.dll）として振る舞うため、PT3用アプリケーション（ptTimer、BonDriver_PT3-STなど）からブルドッグ・チューナーが使用可能になります。  

## 制限事項
以下の制限事項があります。
- AKB-PCR20XはISDB-S 1TSとISDB-T 1TSの合計2TSのため、1台接続時はPT3-SDKの2個目のチューナーは受信不能状態（アンテナが繋がっていない状態と同じ）になります。  
AKB-PCR20Xを2台接続した場合は、PT3と同様にISDB-S 2TSとISDB-T 2TSの合計4TSを同時に受信可能です。（複数台接続の際にシリアルROMのID番号書き換えは不要です。）  
- アプリケーション起動中のUSBの抜き差しには対応していません。
- LNB給電はできません。（そもそもハードウェアに実装されていないため。）

## 内容
リポジトリは以下の構成になっています。
* src\PT3SDKSimulator  
PT3-SDK（SDK_EARTHSOFT_PT3.dll）をシミュレートするDLLです。  
PT3-SDKのインターフェースのうち、最低限必要な機能に絞って実装をしています。  
Microsoft Visual Studio 2017のソリューション形式になっています。

* src\CPLDProgrammer  
CPLDのMAXⅡにロジックを書き込むためのツールです。  
入力ファイルはSVF形式です。  
CUSBWR2.exeがCyUSB3ドライバとの組み合わせで正しく動作するか懸念があったため、独自に書き込みツールを作りました。  
Microsoft Visual Studio 2017のソリューション形式になっています。

* uvision\tuner、uvision\bitbang  
EZ-USB FX2LPのファームウエアです。
tunerはPT3SDKSimulator用、bitbangはCPLDProgrammer用です。  
CY3684 EZ-USB FX2LP Development Kit付属のKeil μVisionのプロジェクト形式になっています。

* quatus  
MAXⅡのロジックです。  
Intel Quatus Primeのプロジェクト形式になっています。

* external  
外部ヘッダーファイル、ライブラリの格納場所です。

## ビルド方法
**32bit版**のDLLおよびアプリケーションの作成手順です。

* PT3-SDKヘッダーファイルの入手と展開  
PT3-SDKのソースコードをダウンロードします。
現時点では[こちら](https://earthsoft.jp/PT3/download_old.html) から「PT3-SDK-300.zip」がダウンロード可能です。  
zipファイル中の「EARTH_PT3.h」「Prefix.h」「Prefix_Bit.h」「Prefix_Integer.h」の4ファイルを「BDTuner\external\inc」に配置します。  
EARTH_PT3.hは、Visual Studio 2017でコンパイルすると「not」という変数名が予約語との競合でエラーになるため、該当行を以下のように書き換えます。  
```c
    //virtual status SetTransferTestMode(ISDB isdb, uint32 tunerIndex, bool testMode = false, uint16 initial = 0, bool not = false/*, bool resetError = false*/) = 0;
    virtual status SetTransferTestMode(ISDB isdb, uint32 tunerIndex, bool testMode = false, uint16 initial = 0, bool _not = false/*, bool resetError = false*/) = 0;
```

* CyAPI3ヘッダーファイルとライブラリの入手と展開  
CyAPI3ライブラリをダウンロードします。
現時点では[こちら](http://www.cypress.com/documentation/software-and-drivers/ez-usb-fx3-software-development-kit) から「cy_ssusbsuite_v1.3.3_0.zip」がダウンロード可能です。  
zipファイル中の「EZ-USB FX3 SDK\1.3\library\cpp\inc」にある「CyAPI.h」「CyUSB30_def.h」の2ファイルを「BDTuner\external\inc」に配置します。  
同様に、zipファイル中の「EZ-USB FX3 SDK\1.3\library\cpp\lib\x86」にある「CyAPI.lib」を「BDTuner\external\lib\x86」に配置します。  

* ビルド  
Visual Studioで「BDTuner\src」にある「BDTuner.sln」を開き、「ソリューションのビルド」を実行します。  
成功すると「BDTuner\src\Release（Debugビルドの場合はBDTuner\src\Debug）」に、「SDK_EARTHSOFT_PT3.dll」と「CPLDProgrammer.exe」が作成されます。

## 導入手順
本ソフトウェアを使用するための導入手順です。
* CyUSB3ドライバの入手とインストール   
CyUSB3ドライバをダウンロードし、使用OSに合ったドライバをインストールします。
現時点では[こちら](https://community.cypress.com/docs/DOC-12366) から「Drivers.zip」がダウンロード可能です。  

* チューナー本体をPCに接続  
AKB-PCR20X本体を付属のUSBケーブルでPCに接続します。

* MAXⅡへロジックを書き込み  
「CPLDProgrammer.exe」を使用してMAXⅡへロジックを書き込みます。  指定するファイルは「BDTuner\quatus\output_files_sample」にある「tuner.svf」です。  
「BDTuner\src\Release」配下でPower Shellまたはコマンドプロンプトを開き、以下のコマンドを実行します。  
`
CPLDProgrammer.exe ..\..\quatus\output_files_sample\tuner.svf
`  
CPLDProgrammerは書き込み速度を追及していないため、完了まで30分ほどかかります。途中経過として実行したSVFの命令が標準出力に出力されます。最後に「finished normal」と出力されれば成功です。  
ロジックを書き込みの際、PCに接続するチューナー本体は**1台のみ**にしてください。

## 使用方法
本ソフトウェアの使用方法です。  
* 基本的には、使用するアプリケーションの.exeファイルがあるディレクトリに「SDK_EARTHSOFT_PT3.dll」を配置すれば使用できるはずです。  
ただし、WindowsのシステムディレクトリにあるPT3-SDKのDLLファイルのみを参照するアプリケーション（ptTimerなど）では「%SystemRoot%\SysWOW64（WindowsOSが32bit版の場合は%SystemRoot%\System32）」に「SDK_EARTHSOFT_PT3.dll」を配置する必要があります。

## 参考資料
作成にあたって、以下のソフトウェアを利用・参照させて頂きました。 ありがとうございます。
* デジ太郎氏 cap_sts_ 2008_03_18版
* アースソフト PT3-SDK ソースコード
* Cypress CY3684 EZ-USB FX2LP Development Kit サンプルコード
* 西田ラヂオ氏 USB CPLD Programmer
