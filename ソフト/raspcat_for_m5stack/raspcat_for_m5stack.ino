/***********************************************************************************************************************
《0》コーディング規約
***********************************************************************************************************************/
/*
【変数接頭語】
bool bl(is) 真偽値（※C言語標準仕様のため参考）
char b 符号付8ビット整数
unsigned char ub 符号なし8ビット整数
short h　符号付16ビット整数
unsigned short uh　符号なし16ビット整数
long w 符号付32ビット整数
unsigned long uw　符号なし32ビット整数

【オリジナル接頭語】
p ピン番号

【定数接頭語】
C 一般定数
P ピン番号(オリジナル)
M マクロ
*/

/***********************************************************************************************************************
《1》取り込みファイル定義
***********************************************************************************************************************/
#include <Dynamixel2Arduino.h>
#include <M5Unified.h>
#include <Wire.h>

/***********************************************************************************************************************
《2》ファイル内でのみ使用する定数定義
***********************************************************************************************************************/
/*モード定義*/

/*ピン番号定義*/
//M5Stack CoreS3 PORT C
const uint8_t P_RX_SERVO = 17;
const uint8_t P_TX_SERVO = 18;

/*Dynamixelサーボ*/
const uint16_t C_DXL_TILT_MINIMUM_POS = 1898;
const uint16_t C_DXL_TILT_MAXIMUM_POS = 2198;
const uint16_t C_DXL_PAN_MINIMUM_POS = 1448;
const uint16_t C_DXL_PAN_MAXIMUM_POS = 2648;
const uint16_t C_DXL_CENTER_POS = 2048;

/*共通定義*/
const uint16_t C_MSEC_INIT_BUZZER_TIME = 200;  //システム起動完了ブザーの時間

const uint16_t C_HIGHTONE_DO = 523;
const uint16_t C_HIGHTONE_RE = 587;
const uint16_t C_HIGHTONE_MI = 659;
const uint16_t C_HIGHTONE_FA = 698;
const uint16_t C_HIGHTONE_SO = 784;
const uint16_t C_HIGHTONE_RA = 880;
const uint16_t C_HIGHTONE_SI = 988;

const uint16_t C_DXL_CENTER_POS = 2048;
const uint8_t TILT_DXL_ID = 1;
const uint8_t PAN_DXL_ID = 2;
const float DXL_PROTOCOL_VERSION = 2.0;

#define DEBUG_SERIAL Serial

/***********************************************************************************************************************
《3》ファイル内でのみ使用するマクロ定義
***********************************************************************************************************************/
/*Nothing*/

/***********************************************************************************************************************
《4》ファイル内でのみ使用するTypedef定義（型定義）
***********************************************************************************************************************/
using namespace ControlTableItem;

/***********************************************************************************************************************
《5》ファイル内でのみ使用するクラス定義
***********************************************************************************************************************/
Dynamixel2Arduino dxl;
HardwareSerial& DXL_SERIAL = Serial1;

/***********************************************************************************************************************
《6》ファイル内で共有するstatic変数定義
***********************************************************************************************************************/
/*Nothing*/

/***********************************************************************************************************************
《7》ファイル内で共有するstatic関数プロトタイプ宣言
***********************************************************************************************************************/
/*Nothing*/

/***********************************************************************************************************************
《8》OS資源定義
***********************************************************************************************************************/
/*Nothing*/

/***********************************************************************************************************************
《9》外部参照変数定義（ドメイン・グローバルな外部変数定義）
***********************************************************************************************************************/
/*Nothing*/

/***********************************************************************************************************************
《10》関数定義
***********************************************************************************************************************/

void setup()
{
    Audio_Setting();
    Dxl_Init();        //シリアルサーボ初期設定
    M5.begin();        //M5スタック初期設定
    Initial_Melody();  //起動音出力

    Wire.begin(M5.Ex_I2C.getSDA(), M5.Ex_I2C.getSCL());
}


void loop()
{
    byte ubTiltData = 1;
    byte ubPanData = 1;
    Debug_Display();
    Wire.requestFrom(0x01, 2);  //2byteリクエスト送信

    while (Wire.available())
    {
        ubTiltData = Wire.read();
        ubPanData = Wire.read();
    }
    M5.Display.println(ubTiltData);
    M5.Display.println(ubPanData);

    //dynamixel
    dxl.setGoalPWM(TILT_DXL_ID, 30.0, UNIT_PERCENT);
    dxl.setGoalPosition(TILT_DXL_ID, Tilt_Random(ubTiltData));
    dxl.setGoalPWM(PAN_DXL_ID, 30.0, UNIT_PERCENT);
    dxl.setGoalPosition(PAN_DXL_ID, Pan_Random(ubTiltData));
    delay(3000);
}
/***********************************************************************************************************************
  関数名 : Get_Dxl_Position
  説明  : サーボ速度取得
  引数  : ubDxlId
  返値  : bRpm
  作成日 : 2024/7/8
  作成者 : S.Yamamoto
***********************************************************************************************************************/
uint16_t uhGet_Dxl_Position(uint8_t ubDxlId)
{
    uint16_t uhPosition = dxl.getPresentPosition(ubDxlId);
    return uhPosition;
}

/***********************************************************************************************************************
  関数名 : Dxl_Init
  説明  : サーボ初期設定
  引数  : None
  返値  : None
  作成日 : 2024/7/8
  作成者 : S.Yamamoto
***********************************************************************************************************************/
void Dxl_Init()
{
    DXL_SERIAL.begin(57600, SERIAL_8N1, RX_SERVO, TX_SERVO);
    dxl = Dynamixel2Arduino(DXL_SERIAL);
    dxl.begin(57600);                                  //通信速度設定
    dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);  //プロトコル設定
    Dxl_Setting(TILT_DXL_ID);                          //ティルト用初期設定
    Dxl_Setting(PAN_DXL_ID);                           //パン用初期設定
}

/***********************************************************************************************************************
  関数名 : Debug_Display
  説明  : デバッグ用ディスプレイ表示
  引数  : None
  返値  : None
  作成日 : 2024/7/30
  作成者 : S.Yamamoto
***********************************************************************************************************************/
void Debug_Display()
{
    M5.Display.startWrite();
    M5.Display.fillScreen(BLACK);
    M5.Display.setCursor(0, 0);

    M5.Display.printf("I2C Scan\n");
    M5.Display.printf("SDA:%2d SCL:%2d\n", M5.Ex_I2C.getSDA(), M5.Ex_I2C.getSCL());
}

/***********************************************************************************************************************
  関数名 : Dxl_Init_Setting
  説明  : サーボ初期設定
  引数  : ubDxlId
  返値  : None
  作成日 : 2024/7/8
  作成者 : S.Yamamoto
***********************************************************************************************************************/
void Dxl_Setting(uint8_t ubDxlId)
{
    dxl.ping(ubDxlId);
    dxl.torqueOff(ubDxlId);
    dxl.setOperatingMode(ubDxlId, OP_POSITION);  //ポジション指定モード
    dxl.torqueOn(ubDxlId);
    dxl.ledOn(ubDxlId);
    dxl.setGoalPosition(ubDxlId, C_DXL_CENTER_POS);  //サーボ初期位置へ移動
}

/***********************************************************************************************************************
  関数名 : Initial_Melody
  説明  : 起動音
  引数  : None
  返値  : None
  作成日 : 2024/7/8
  作成者 : S.Yamamoto
***********************************************************************************************************************/
void Initial_Melody()
{
    M5.Speaker.tone(C_HIGHTONE_DO, C_MSEC_INIT_BUZZER_TIME);  //ド
    delay(200);
    M5.Speaker.tone(C_HIGHTONE_RE, C_MSEC_INIT_BUZZER_TIME);  //レ
    delay(200);
    M5.Speaker.tone(C_HIGHTONE_MI, C_MSEC_INIT_BUZZER_TIME);  //ミ
    delay(200);
    M5.Speaker.end();  //起動音停止（必要）
}

/***********************************************************************************************************************
  関数名 : Audio_Setting
  説明  : スピーカー初期設定
  引数  : None
  返値  : None
  作成日 : 2024/7/25
  作成者 : S.Yamamoto
***********************************************************************************************************************/
void Audio_Setting()
{
    auto config = M5.Speaker.config();
    config.sample_rate = 44100;
    M5.Speaker.config(config);
    M5.Speaker.setVolume(150);
    M5.Speaker.begin();
}

/***********************************************************************************************************************
  関数名 : Tilt_Random
  説明  : 乱数
  引数  : ubDirection
  返値  : uhRandom
  作成日 : 2024/7/25
  作成者 : S.Yamamoto
***********************************************************************************************************************/
uint16_t Tilt_Random(uint8_t ubDirection)
{
    uint16_t uhRandom = 0;
    if (ubDirection == 2)
    {
        uhRandom = random(C_DXL_CENTER_POSITION_VALUE + 50, C_DXL_TILT_MAXIMUM_POS);
    }
    else if (ubDirection == 1)
    {
        uhRandom = random(C_DXL_CENTER_POSITION_VALUE - 50, C_DXL_CENTER_POSITION_VALUE + 50);
    }
    else if (ubDirection == 0)
    {
        uhRandom = random(C_DXL_TILT_MINIMUM_POS, C_DXL_CENTER_POSITION_VALUE - 50);
    }

    return uhRandom;
}

/***********************************************************************************************************************
  関数名 : Pan_Random
  説明  : 乱数
  引数  : ubDirection
  返値  : uhRandom
  作成日 : 2024/7/25
  作成者 : S.Yamamoto
***********************************************************************************************************************/
uint16_t Pan_Random(uint8_t ubDirection)
{
    uint16_t uhRandom = 0;
    if (ubDirection == 2)
    {
        uhRandom = random(C_DXL_CENTER_POS + 200, C_DXL_PAN_MAXIMUM_POS);
    }
    else if (ubDirection == 1)
    {
        uhRandom = random(C_DXL_CENTER_POS - 200, C_DXL_CENTER_POS + 200);
    }
    else if (ubDirection == 0)
    {
        uhRandom = random(C_DXL_PAN_MINIMUM_POS, C_DXL_CENTER_POS - 200);
    }

    return uhRandom;
}