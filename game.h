/*********************************************************************
* 【数取マインスイーパー】サーバーサイド対戦関数
*
* 基本設計書第0.04版・詳細設計書第.0.50版参照
* ソケット通信機能と盤面機能はヘッダファイルにて共通化
*
* 関数名：startGame
*
* 引数　：なし
*
* 戻り値：int 0
*
* 作成日:2024/07/18
* 作成者:浦野 稚南美
*
*********************************************************************/
#ifndef GAME_H
#define GAME_H

#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>

#define SIZE 9								/* 盤面の数(9×9）*/
#define WINNING_SCORE 10					/* 勝利点数 */
#define LOSING_SCORE -20					/* 敗北点数 */
#define NAME 20								/* プレイヤー名の制限文字数 */
#define FIRST_SECOND 30						/* 先攻後攻のメッセージ表示の文字数 */
#define RESULT_FILE "Result.csv"			/* 結果ファイル */
#define TEMP_FILE "temp_result.csv"			/* 結果ファイル記入時の添付ファイル */
#define MAX_RECORDS 10						/* 結果ファイル格納件数の最大値 */


/* プロトタイプ宣言 */
int startGame( void );														/* サーバー側の対戦用関数 */
void getTime( char *buffer , int size );									/* ゲーム開始日時の取得用関数 */
int orderFirst( void );														/* 先攻後攻決定用関数 */
void makeFile( char* server_name , char* client_name , 
					int my_score , int other_score , char* dataTime);		/* 対戦履歴(結果ファイル)編集用関数 */

#endif


