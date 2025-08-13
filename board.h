/*********************************************************************
* 【数取マインスイーパー】盤面処理ヘッダファイル
*
* 基本設計書第0.04版・詳細設計書第.0.50版参照
*
* 
* 作成日:2024/07/10
*
*********************************************************************/
#ifndef BOARD_H		// BOARD_Hが未定義なら↓
#define BOARD_H		// BOARD_Hを定義

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>

// マクロ定義
#define SIZE 9

/* 関数プロトタイプ宣言 */
void setBoard( int board[SIZE][SIZE], int bombs[SIZE][SIZE], int open[SIZE][SIZE] );
void displayBoard( int board[SIZE][SIZE], int bombs[SIZE][SIZE], int open[SIZE][SIZE] );
int chooseCell( int board[SIZE][SIZE], int bombs[SIZE][SIZE], int open[SIZE][SIZE], int x, int y );

#endif // BOARD_H


