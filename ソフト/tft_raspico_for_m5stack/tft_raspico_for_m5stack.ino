/***********************************************************************************************************************
《0》コーディング規約
***********************************************************************************************************************/
/*
【変数接頭語】
bool bl 真偽値
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
P ピン番号
M マクロ

【左目ピン配置】
GND<->GND
VCC<->3V3
SCL<->GP18(common)
SDA<->GP19(common)
RES<->GP22(common)
DC<->GP28(common)
CS<->GP16
BL<->NONE

【右目ピン配置】
GND<->GND
VCC<->3V3
SCL<->GP18(common)
SDA<->GP19(common)
RES<->GP22(common)
DC<->GP28(common)
CS<->GP13
BL<->NONE
*/

/***********************************************************************************************************************
《1》取り込みファイル定義
***********************************************************************************************************************/
#ifndef __eyeStorage_
#include "eyeStorage.h"
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Servo.h>
#include <Wire.h>
#endif

/***********************************************************************************************************************
《2》ファイル内でのみ使用する定数定義
***********************************************************************************************************************/
/*ピン設定*/
#define P_CS_EYE_LEFT (16)
#define P_CS_EYE_RIGHT (13)
#define P_DEBUG_LED (14)
#define P_TOUCH_SENSOR (10)
#define P_LIGHT_SENSOR (26)
#define P_TAIL_SERVO (9)

//共通定義
const bool C_ENABLE = true;
const bool C_DISABLE = false;
const bool C_ON = true;
const bool C_OFF = false;
const bool C_LEFT_EYE = false;
const bool C_RIGHT_EYE = true;
const uint8_t C_BLINK_CLOSE = 1;
const uint8_t C_BLINK_OPEN = 2;
const uint8_t C_BLINK_STOP = 0;

const uint8_t C_EYE_SIZE = 100;         //目のサイズ
const uint8_t C_NORMAL_EYE_WIDTH = 35;  //瞳孔の幅
const uint8_t C_NORMAL_EYE = 0;         //標準目
const uint8_t C_CLOSED_EYE = 1;         //閉じ目
const uint32_t C_BLINK_TIME = 5000;     //瞬きの時間
const uint8_t C_PUPIL_HEIGHT = 35;      //瞳孔高さ

enum wag_speed
{
    NO_MOVEMENT,
    NORMAL_SPEED,
    VARIABLE_SPEED,
};

//extern const unsigned short uhLeftEye[];
//extern const unsigned short uhRightEye[];

/***********************************************************************************************************************
《3》ファイル内でのみ使用するマクロ定義
***********************************************************************************************************************/

/***********************************************************************************************************************
《4》ファイル内でのみ使用するTypedef定義（型定義）
***********************************************************************************************************************/

/***********************************************************************************************************************
《5》ファイル内でのみ使用するクラス定義
***********************************************************************************************************************/
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprEyeBase = TFT_eSprite(&tft);
TFT_eSprite sprEyePupil = TFT_eSprite(&tft);
Servo tailservo;

/***********************************************************************************************************************
《6》ファイル内で共有するstatic変数定義
***********************************************************************************************************************/
static uint8_t _gubBlinkDir = C_BLINK_STOP;
static int8_t _gbEyeX = 0;
static int8_t _gbEyeY = 0;
static bool _gblClosedEyeFlag = false;

/***********************************************************************************************************************
《7》ファイル内で共有するstatic関数プロトタイプ宣言
***********************************************************************************************************************/

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

/*****************************************************************************************
  関数名 : setup
  説明  : 初期設定
  引数  : None
  返値  : None
  作成日 : 2024/5/10
  作成者 : S.Yamamoto
*****************************************************************************************/
void setup(void)
{
    //通信設定
    Wire.setSDA(0);
    Wire.setSCL(1);
    Wire.begin(0x01);
    Wire.onRequest(requestEvent);  //I2C

    //ピン設定
    pinMode(P_CS_EYE_LEFT, OUTPUT);   //左ディスプレイのチップセレクトピン出力定義
    pinMode(P_CS_EYE_RIGHT, OUTPUT);  //右ディスプレイのチップセレクトピン出力定義
    pinMode(P_DEBUG_LED, OUTPUT);     //デバッグLEDのピンを出力設定
    pinMode(P_TOUCH_SENSOR, INPUT);   //静電容量センサのピンを入力設定

    _gubBlinkDir = C_BLINK_CLOSE;  //瞬きの変数フラグを立てる

    //しっぽサーボモータ初期設定
    tailservo.attach(P_TAIL_SERVO);
    tailservo.write(60);
    delay(100);

    EyeInit();    //アイディスプレイの初期化
    LedFlash(2);  //デバッグLED点滅
    LedDrive(C_ON);
}

/*****************************************************************************************
  関数名 : loop
  説明  : メインループ
  引数  : None
  返値  : None
  作成日 : 2024/5/10
  作成者 : S.Yamamoto
*****************************************************************************************/
void loop()
{
    EyeDrive(_gbEyeX, _gbEyeY);
    TailMovement();
}

/*****************************************************************************************
  関数名 : requestEvent
  説明  : i2c送信タスク
  引数  : None
  返値  : None
  作成日 : 2024/7/28
  作成者 : S.Yamamoto
*****************************************************************************************/
void requestEvent()
{
    Wire.write(Tilt_Random());  //1~3
    Wire.write(Pan_Random());   //1~3
    _gbEyeY = Tilt_Random();
    _gbEyeX = Pan_Random();
}

/***********************************************************************************************************************
  関数名 : Tilt_Random
  説明  : 乱数
  引数  : None
  返値  : uhRandom
  作成日 : 2024/7/25
  作成者 : S.Yamamoto
***********************************************************************************************************************/
uint8_t Tilt_Random()
{
    uint8_t uhRandom = random(0, 3);
    return uhRandom;
}

/***********************************************************************************************************************
  関数名 : Pan_Random
  説明  : 乱数
  引数  : None
  返値  : uhRandom
  作成日 : 2024/7/25
  作成者 : S.Yamamoto
***********************************************************************************************************************/
uint8_t Pan_Random()
{
    uint8_t uhRandom = random(0, 3);
    return uhRandom;
}

/*****************************************************************************************
  関数名 : EyeInit
  説明  : 左右アイディスプレイの初期化
  引数  : None
  返値  : None
  作成日 : 2024/5/10
  作成者 : S.Yamamoto
*****************************************************************************************/
void EyeInit()
{
    digitalWrite(P_CS_EYE_LEFT, LOW);   //左ディスプレイセット
    digitalWrite(P_CS_EYE_RIGHT, LOW);  //右ディスプレイセット
    tft.setRotation(2);                 //両ディスプレイ180度
    tft.init();                         //両ディスプレイリセット
    tft.fillScreen(TFT_BLACK);          //両ディスプレイを黒背景に設定
}

/*****************************************************************************************
  関数名 : LedFlash
  説明  : デバッグLEDの点滅
  引数  : ubCount
  返値  : None
  作成日 : 2024/5/10
  作成者 : S.Yamamoto
*****************************************************************************************/
void LedFlash(uint8_t ubCountEnd)
{
    for (uint8_t i = 0; i < ubCountEnd; i++)
    {
        digitalWrite(P_DEBUG_LED, HIGH);
        delay(100);
        digitalWrite(P_DEBUG_LED, LOW);
        delay(100);
    }
}

/*****************************************************************************************
  関数名 : LedDrive
  説明  : LEDの点灯点滅
  引数  : isState
  返値  : None
  作成日 : 2024/7/27
  作成者 : S.Yamamoto
*****************************************************************************************/
void LedDrive(bool isState)
{
    digitalWrite(P_DEBUG_LED, isState);
}

/*****************************************************************************************
  関数名 : EyeDrive
  説明  : 左右アイディスプレイの表示
  引数  : bPosX, bPosY, ubPupilWidth, ubEyeShape
  返値  : None
  作成日 : 2024/5/10
  作成者 : S.Yamamoto
*****************************************************************************************/
void EyeDrive(int8_t bPosX, int8_t bPosY)
{
    static uint8_t ubEyelidHeight = 0;  //まぶたの高さ
    static uint32_t uwOldTime = 0;      //瞬き時間

    //瞬き
    uint32_t uwNowTime = millis();      //現在時刻の保持
    if (_gubBlinkDir == C_BLINK_CLOSE)  //閉じる方向
    {
        if (ubEyelidHeight < 100)
        {
            ubEyelidHeight += 20;
        }
        else
        {
            _gubBlinkDir = C_BLINK_OPEN;
        }
    }
    else if (_gubBlinkDir == C_BLINK_OPEN)  //開ける方向
    {
        if (ubEyelidHeight > 0)
        {
            ubEyelidHeight -= 20;
        }
        else
        {
            _gubBlinkDir = C_BLINK_STOP;
        }
    }
    if (_gubBlinkDir == 0 && (uwNowTime - uwOldTime) >= C_BLINK_TIME)  //瞬き間隔が経過したら
    {
        _gubBlinkDir = C_BLINK_CLOSE;
        uwOldTime = uwNowTime;
    }

    //右目表示
    LeftEyeStatus(C_DISABLE);
    NormalEyeDesign(C_RIGHT_EYE, bPosX, bPosY, ubEyelidHeight);
    LeftEyeStatus(C_ENABLE);

    //左目表示
    RightEyeStatus(C_DISABLE);
    NormalEyeDesign(C_LEFT_EYE, bPosX, bPosY, ubEyelidHeight);
    RightEyeStatus(C_ENABLE);
}

/*****************************************************************************************
  関数名 : NormalEyeDesign
  説明  : 普通目のデザイン
  引数  : blSide
  返値  : None
  作成日 : 2024/5/10
  作成者 : S.Yamamoto
*****************************************************************************************/
void NormalEyeDesign(bool blSide, int8_t bPosX, int8_t bPosY, uint8_t ubEyelidHeight)
{
    sprEyeBase.createSprite(C_EYE_SIZE, C_EYE_SIZE);  //目のベーススプライト生成
    if (blSide == C_LEFT_EYE)                         //左目選択なら
    {
        if (bPosY == 0)
        {
            if (bPosX == 0) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY2X1);
            else if (bPosX == 1) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY2X2);
            else if (bPosX == 2) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY2X3);
        }
        else if (bPosY == 1)
        {
            if (bPosX == 0) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY3X1);
            else if (bPosX == 1) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY3X2);
            else if (bPosX == 2) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY3X3);
        }
        else if (bPosY == 2)
        {
            if (bPosX == 0) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY4X1);
            else if (bPosX == 1) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY4X2);
            else if (bPosX == 2) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY4X3);
        }
    }
    else  //右目選択なら
    {
        if (bPosY == 0)
        {
            if (bPosX == 0) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY2X3);
            else if (bPosX == 1) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY2X4);
            else if (bPosX == 2) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY2X5);
        }
        else if (bPosY == 1)
        {
            if (bPosX == 0) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY3X3);
            else if (bPosX == 1) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY3X4);
            else if (bPosX == 2) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY3X5);
        }
        else if (bPosY == 2)
        {
            if (bPosX == 0) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY4X3);
            else if (bPosX == 1) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY4X4);
            else if (bPosX == 2) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY4X5);
        }
    }

    sprEyeBase.fillRect(0, 0, C_EYE_SIZE, ubEyelidHeight, TFT_BLACK);                            //まぶた表示
    sprEyeBase.pushSprite(tft.width() / 2 - C_EYE_SIZE / 2, tft.height() / 2 - C_EYE_SIZE / 2);  //スプライト出力(不動)

    sprEyeBase.deleteSprite();
}

/*****************************************************************************************
  関数名 : TailMovement
  説明  : しっぽの動作
  引数  : None
  返値  : None
  作成日 : 2024/8/1
  作成者 : S.Yamamoto
*****************************************************************************************/
void TailMovement()
{
    uint8_t ubTailPosition = 0;
    uint16_t uhRandom = random(0, 3);  //０：動かない、１：動く、２：
    //しっぽ動作連動
    if (uhRandom == 0)
    {
        //do nothing
    }
    else if (uhRandom == 1)  //通常揺動
    {
        for (uint8_t ubPos = 30; ubPos <= 150; ubPos += 10)
        {
            tailservo.write(ubPos);
            delay(5);
        }
        for (uint8_t ubPos = 150; ubPos >= 30; ubPos -= 10)
        {
            tailservo.write(ubPos);
            delay(5);
        }
    }

    else if (uhRandom == 1)  //通常揺動
    {
        for (uint8_t ubPos = 30; ubPos <= 150; ubPos += 10)
        {
            tailservo.write(ubPos);
            delay(5);
        }
        for (uint8_t ubPos = 150; ubPos >= 30; ubPos -= 10)
        {
            tailservo.write(ubPos);
            delay(2);
        }
    }
}

/*****************************************************************************************
  関数名 : GetLightValue
  説明  : 照度センサの数値取得
  引数  : None
  返値  : value
  作成日 : 2024/5/10
  作成者 : S.Yamamoto
*****************************************************************************************/
uint16_t GetLightValue()
{
    uint16_t ubSensorOutput = 0;
    for (uint8_t i = 0; i < 5; i++)
    {
        uint16_t ubLightSensorValue = analogRead(P_LIGHT_SENSOR);
        if (ubLightSensorValue > 50)  //ノイズ除去
        {
            ubLightSensorValue = 50;
        }
        else if (ubLightSensorValue < 20)  //ノイズ除去
        {
            ubLightSensorValue = 20;
        }
        ubLightSensorValue = map(ubLightSensorValue, 20, 50, 35, 10);
        ubSensorOutput += ubLightSensorValue;
    }

    //Serial.println(ubLightSensorValue);
    ubSensorOutput /= 5;
    Serial.println(ubSensorOutput);
    return ubSensorOutput;  //平均値
    //return 35;
}

/*****************************************************************************************
  関数名 : LeftEyeStatus
  説明  : 左アイディスプレイの切り替え
  引数  : blSelective
  返値  : None
  作成日 : 2024/5/10
  作成者 : S.Yamamoto
*****************************************************************************************/
void LeftEyeStatus(bool blSelective)
{
    if (blSelective == true)
    {
        digitalWrite(P_CS_EYE_LEFT, LOW);
    }
    else
    {
        digitalWrite(P_CS_EYE_LEFT, HIGH);
    }
}

/*****************************************************************************************
  関数名 : RightEyeStatus
  説明  : 右アイディスプレイの切り替え
  引数  : blSelective
  返値  : None
  作成日 : 2024/5/10
  作成者 : S.Yamamoto
*****************************************************************************************/
void RightEyeStatus(bool blSelective)
{
    if (blSelective == true)
    {
        digitalWrite(P_CS_EYE_RIGHT, LOW);
    }
    else
    {
        digitalWrite(P_CS_EYE_RIGHT, HIGH);
    }
}
