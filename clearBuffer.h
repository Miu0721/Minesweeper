/*********************************************************************
* 【数取マインスイーパー】バッファクリア
*
* 基本設計書第0.10版・詳細設計書第.0.70版参照
*
* 通信待機時の誤入力処理を行う関数、誤入力がない場合はタイムアウトで入力
* 何も誤入力がに場合の画面待機をタイムアウトさせる
* 相手から何らかの影響で接続が切れた際に、アラートを出して終了させる　
* 
* 作成日:2024/07/18
*
*********************************************************************/

#ifndef CLEARBUFFER_H
#define CLEARBUFFER_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/select.h>
#include <errno.h>
#include <unistd.h>

#define TIMEOUT_SEC 0
#define TIMEOUT_MIC 100000

// 関数のプロトタイプ宣言
void clearBuffer(int sock);
void clearBuf();

#endif


