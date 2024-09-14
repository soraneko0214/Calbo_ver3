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
#ifndef __eyeAnimation_
#include "eyeAnimation.h"
#endif
#ifndef __blinkAnimation_
#include "blinkAnimation.h"
#endif

#include <SPI.h>
#include <TFT_eSPI.h>
#include <Wire.h>


/***********************************************************************************************************************
《2》ファイル内でのみ使用する定数定義
***********************************************************************************************************************/
/*ピン設定*/
#define P_CS_EYE_LEFT (16)
#define P_CS_EYE_RIGHT (13)

#define P_DEBUG_LED (14)
#define P_TOUCH_SENSOR (10)
#define P_LIGHT_SENSOR (26)

#define P_STEPPING1 (9)
#define P_STEPPING2 (8)
#define P_STEPPING3 (7)
#define P_STEPPING4 (6)

//共通定義
const bool C_ENABLE = true;
const bool C_DISABLE = false;
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
TFT_eSprite sprEyeBrow = TFT_eSprite(&tft);
TFT_eSprite sprEyePupil = TFT_eSprite(&tft);

/***********************************************************************************************************************
《6》ファイル内で共有するstatic変数定義
***********************************************************************************************************************/
static uint8_t _gubBlinkDir = C_BLINK_STOP;
static bool _gbTailFlag = true;
static int8_t _gbEyeX = 0;
static int8_t _gbEyeY = 0;

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
    Wire.onRequest(requestEvent);

    //Serial.begin(9600);  //デバッグ用通信速度

    //ピン設定
    pinMode(P_CS_EYE_LEFT, OUTPUT);   //左ディスプレイのチップセレクトピン出力定義
    pinMode(P_CS_EYE_RIGHT, OUTPUT);  //右ディスプレイのチップセレクトピン出力定義
    pinMode(P_DEBUG_LED, OUTPUT);     //デバッグLEDのピンを出力設定
    pinMode(P_TOUCH_SENSOR, INPUT);   //静電容量センサのピンを入力設定


    pinMode(P_STEPPING1, OUTPUT);
    pinMode(P_STEPPING2, OUTPUT);
    pinMode(P_STEPPING3, OUTPUT);
    pinMode(P_STEPPING4, OUTPUT);

    _gubBlinkDir = C_BLINK_CLOSE;  //瞬きの変数フラグを立てる

    EyeInit();  //アイディスプレイの初期化
    tft.fillScreen(TFT_WHITE);
    tft.pushImage(0, 70, 128, 20, uhLogo);  //ロゴをアイディスプレイに表示
    delay(1500);                            //プロモーションタイム

    LedFlash(2);  //デバッグLED点滅
    EyeInit();
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
    if (_gubBlinkDir == C_BLINK_STOP && _gbTailFlag == true)
    {
        TailDrive();
        _gbTailFlag = false;
    }
    GetTouchSensor();  //静電容量センサデバッグ用
}

/*****************************************************************************************
  関数名 : TailDrive
  説明  : しっぽ動作
  引数  : None
  返値  : None
  作成日 : 2024/8/12
  作成者 : S.Yamamoto
*****************************************************************************************/
void TailDrive()
{
    uint8_t ubRandom = random(0, 3);
    SteppingMotorDrive(4096);
    /*
    if (ubRandom == 0)
    {
    }
    else
    {
        //do nothing
    }
    */
}

/*****************************************************************************************
  関数名 : SteppingMotorDrive
  説明  : ステッピングモータ動作
  引数  : uhStepCountEnd
  返値  : None
  作成日 : 2024/9/9
  作成者 : S.Yamamoto
*****************************************************************************************/
void SteppingMotorDrive(uint16_t uhStepCountEnd)
{
    //4096 Step で360degree
    uint16_t uhCount = 0;
    for (uhCount = 0; uhCount < uhStepCountEnd; uhCount++)
    {
        //i を割った余りを評価。つまり(0, 1, 2, 3, 4, 5, 6, 7のいずれか)
        switch (uhCount % 8)
        {
            case 0:
                digitalWrite(P_STEPPING1, HIGH);
                digitalWrite(P_STEPPING2, LOW);
                digitalWrite(P_STEPPING3, LOW);
                digitalWrite(P_STEPPING4, LOW);
                break;

            case 1:
                digitalWrite(P_STEPPING1, HIGH);
                digitalWrite(P_STEPPING2, HIGH);
                digitalWrite(P_STEPPING3, LOW);
                digitalWrite(P_STEPPING4, LOW);
                break;

            case 2:
                digitalWrite(P_STEPPING1, LOW);
                digitalWrite(P_STEPPING2, HIGH);
                digitalWrite(P_STEPPING3, LOW);
                digitalWrite(P_STEPPING4, LOW);
                break;

            case 3:
                digitalWrite(P_STEPPING1, LOW);
                digitalWrite(P_STEPPING2, HIGH);
                digitalWrite(P_STEPPING3, HIGH);
                digitalWrite(P_STEPPING4, LOW);
                break;

            case 4:
                digitalWrite(P_STEPPING1, LOW);
                digitalWrite(P_STEPPING2, LOW);
                digitalWrite(P_STEPPING3, HIGH);
                digitalWrite(P_STEPPING4, LOW);
                break;

            case 5:
                digitalWrite(P_STEPPING1, LOW);
                digitalWrite(P_STEPPING2, LOW);
                digitalWrite(P_STEPPING3, HIGH);
                digitalWrite(P_STEPPING4, HIGH);
                break;

            case 6:
                digitalWrite(P_STEPPING1, LOW);
                digitalWrite(P_STEPPING2, LOW);
                digitalWrite(P_STEPPING3, LOW);
                digitalWrite(P_STEPPING4, HIGH);
                break;

            case 7:
                digitalWrite(P_STEPPING1, HIGH);
                digitalWrite(P_STEPPING2, LOW);
                digitalWrite(P_STEPPING3, LOW);
                digitalWrite(P_STEPPING4, HIGH);
                break;
        }

        delayMicroseconds(1000);  //Step ウエイト
    }

    //1週ごとのウエイト
    delay(1000);
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
    uint8_t ubPosY = Tilt_Random();
    uint8_t ubPosX = Pan_Random();
    Wire.write(ubPosY);  //0~199
    Wire.write(ubPosX);  //0~199
    _gbEyeY = ManualMap(ubPosY);
    _gbEyeX = ManualMap(ubPosX);
}

/*****************************************************************************************
  関数名 : ManualMap
  説明  : 手動マップ
  引数  : None
  返値  : None
  作成日 : 2024/8/11
  作成者 : S.Yamamoto
*****************************************************************************************/
int8_t ManualMap(uint8_t ubPos)
{
    int8_t bPos = 0;
    if (ubPos >= 0 && ubPos < 60)
    {
        bPos = -1;
    }
    else if (ubPos >= 60 && ubPos < 140)
    {
        bPos = 0;
    }
    else if (ubPos >= 140 && ubPos < 200)
    {
        bPos = 1;
    }
    //Serial.println(bPos);
    return bPos;
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
    uint16_t ubRandom = random(0, 200);
    return ubRandom;
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
    uint8_t ubRandom = random(0, 200);
    return ubRandom;
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
    static uint8_t ubTailPosition = 0;

    static uint16_t uhRandomNumber = 0;
    uhRandomNumber = random(3000, 7000);
    //瞬き
    uint32_t uwNowTime = millis();      //現在時刻の保持
    if (_gubBlinkDir == C_BLINK_CLOSE)  //閉じる方向
    {
        if (ubEyelidHeight < 120)
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
        _gbTailFlag = true;
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
    int8_t bHeartPupilPosX = 0;
    int8_t bHeartPupilPosY = 0;

    sprEyeBase.createSprite(C_EYE_SIZE, C_EYE_SIZE);  //目のベーススプライト生成
    if (blSide == C_LEFT_EYE)                         //左目選択なら
    {
        if (bPosY == -1)
        {
            if (bPosX == -1) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY2X1);
            else if (bPosX == 0) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY2X2);
            else if (bPosX == 1) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY2X3);
        }
        else if (bPosY == 0)
        {
            if (bPosX == -1) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY3X1);
            else if (bPosX == 0) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY3X2);
            else if (bPosX == 1) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY3X3);
        }
        else if (bPosY == 1)
        {
            if (bPosX == -1) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY4X1);
            else if (bPosX == 0) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY4X2);
            else if (bPosX == 1) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY4X3);
        }
        bHeartPupilPosX = bHeartPupilPosX + bPosX * 5 - 5;
    }
    else  //右目選択なら
    {
        if (bPosY == -1)
        {
            if (bPosX == -1) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY2X3);
            else if (bPosX == 0) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY2X4);
            else if (bPosX == 1) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY2X5);
        }
        else if (bPosY == 0)
        {
            if (bPosX == -1) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY3X3);
            else if (bPosX == 0) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY3X4);
            else if (bPosX == 1) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY3X5);
        }
        else if (bPosY == 1)
        {
            if (bPosX == -1) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY4X3);
            else if (bPosX == 0) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY4X4);
            else if (bPosX == 1) sprEyeBase.pushImage(0, 0, 100, 100, uhEyeY4X5);
        }
        bHeartPupilPosX = bHeartPupilPosX + bPosX * 5 + 5;
    }
    bHeartPupilPosY = bHeartPupilPosY + -1 * bPosY * 5;
    if (GetTouchSensor())  //眉間をなでられたら
    {
        sprEyePupil.createSprite(30, 25);  //目のベーススプライト生成
        sprEyePupil.pushImage(0, 0, 30, 25, uhHeart);
        sprEyePupil.pushToSprite(&sprEyeBase, 50 - 15 + bHeartPupilPosX, 50 - 12 + bHeartPupilPosY, TFT_BLACK);
    }

    BlinkAnimation(ubEyelidHeight, blSide);

    sprEyeBase.pushSprite(tft.width() / 2 - C_EYE_SIZE / 2, tft.height() / 2 - C_EYE_SIZE / 2);  //スプライト出力(不動)

    sprEyeBrow.deleteSprite();
    sprEyeBase.deleteSprite();
}

/*****************************************************************************************
  関数名 : BlinkAnimation
  説明  : まばたき
  引数  : ubEyelidHeight, blSide
  返値  : None
  作成日 : 2024/8/11
  作成者 : S.Yamamoto
*****************************************************************************************/
void BlinkAnimation(uint8_t ubEyelidHeight, bool blSide)
{
    uint8_t ubBlinkScene = ubEyelidHeight / 20;
    sprEyeBrow.createSprite(C_EYE_SIZE, C_EYE_SIZE);  //目のベーススプライト生成
    if (ubBlinkScene == 0)
    {
        //do nothing
    }
    else if (ubBlinkScene == 1)
    {
        if (blSide == C_LEFT_EYE) sprEyeBrow.pushImage(0, 0, 100, 100, uhBlinkLeft1);
        else sprEyeBrow.pushImage(0, 0, 100, 100, uhBlinkRight1);
    }
    else if (ubBlinkScene == 2)
    {
        if (blSide == C_LEFT_EYE) sprEyeBrow.pushImage(0, 0, 100, 100, uhBlinkLeft2);
        else sprEyeBrow.pushImage(0, 0, 100, 100, uhBlinkRight2);
    }
    else if (ubBlinkScene == 3)
    {
        if (blSide == C_LEFT_EYE) sprEyeBrow.pushImage(0, 0, 100, 100, uhBlinkLeft3);
        else sprEyeBrow.pushImage(0, 0, 100, 100, uhBlinkRight3);
    }
    else if (ubBlinkScene == 4)
    {
        if (blSide == C_LEFT_EYE) sprEyeBrow.pushImage(0, 0, 100, 100, uhBlinkLeft4);
        else sprEyeBrow.pushImage(0, 0, 100, 100, uhBlinkRight4);
    }
    else if (ubBlinkScene == 5)
    {
        if (blSide == C_LEFT_EYE) sprEyeBrow.pushImage(0, 0, 100, 100, uhBlinkLeft5);
        else sprEyeBrow.pushImage(0, 0, 100, 100, uhBlinkRight5);
    }
    else
    {
        sprEyeBrow.fillRect(0, 0, 100, 100, TFT_BLACK);
    }
    if (ubBlinkScene != 0)
    {
        sprEyeBrow.pushToSprite(&sprEyeBase, 0, 0, TFT_WHITE);
    }
    //Serial.println(ubBlinkScene);
}

/*****************************************************************************************
  関数名 : GetTouchSensor
  説明  : 静電容量センサの数値取得
  引数  : None
  返値  : value
  作成日 : 2024/8/10
  作成者 : S.Yamamoto
*****************************************************************************************/
bool GetTouchSensor()
{
    bool blSensorOutput = false;
    if (digitalRead(P_TOUCH_SENSOR) == true)
    {
        blSensorOutput = true;
        LedDrive(true);
    }
    else
    {
        LedDrive(false);
    }
    return blSensorOutput;
}


/*****************************************************************************************
  関数名 : GetLightValue
  説明  : 照度センサの数値取得
  引数  : None
  返値  : ubSensorOutput
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
