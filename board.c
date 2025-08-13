/*********************************************************************
* 【数取マインスイーパー】盤面処理ヘッダファイル
*
* 基本設計書第0.04版・詳細設計書第.0.50版参照
*
* 
* 作成日:2024/07/10
* 作成者:秋山 克成 / 浦野 稚南美
*
*********************************************************************/

#include "board.h"
#define SIZE 9

/*****************************************************************************
* 関数名：setBoard
*
* 内容　：引数で受け取った配列に1～9までの数字を配置、
*		同様に引数で受け取った爆弾用配列に事前に設定した確率をもとに爆弾を設置。
*
* 引数　：int board[SIZE][SIZE] , int bombs[SIZE][SIZE] , int open[SIZE][SIZE]
*
* 戻り値：なし
******************************************************************************/
void setBoard( int board[SIZE][SIZE], int bombs[SIZE][SIZE], int open[SIZE][SIZE] )
{
	int i, j , k;		/* カウンタ用変数 */
	int value;			/* 1～9までの数値を判別する用変数 */
	int bombProb;		/* 爆弾の確率 */
	int random;			/* 乱数の戻り値格納用変数 */
	int num[SIZE][SIZE];/* 乱数格納用配列 */
	bool flag;			/* 乱数がすでに出ている数値かどうか判定用変数 */
	
	/* 乱数の初期化 */
	srand( time(NULL) );
	
	for( i = 0; i < SIZE; i++ )
	{
		for( j = 0; j < SIZE; j++ )
		{
			board[i][j] = (i % SIZE) + 1;		/* 1～9までの数字を順番に割り当てる */
			bombs[i][j] = 0;				/* 爆弾の初期化 */
			open[i][j] = 0;					/* めくった状態の初期化 */
			num[i][j] = 0;					/* 乱数格納用配列の初期化 */
		}
	}
	
	/* 爆弾を配置 */
	for( i = 0; i < SIZE; i++ )
	{
		for( j = 0; j < SIZE; j++ )
		{
			value = board[i][j];
			bombProb = 0;
			flag = false;
			
			/* 1～9(y軸)を元に爆弾の確率を決定 */
			switch( value )
			{
			case 1:
			case 2:
				bombProb = 2;			/* 1・2の場合は2/9の確率で爆弾 */
				break;
			case 3:
			case 4:
				bombProb = 3;			/* 3・4の場合は3/9の確率で爆弾 */
				break;
			case 5:
			case 6:
				bombProb = 5;			/* 5・6の場合は5/9の確率で爆弾 */
				break;
			case 7:
			case 8:
				bombProb = 6;			/* 7・8の場合は6/9の確率で爆弾 */
				break;
			case 9:
				bombProb = 7;			/* 9の場合は7/9の確率で爆弾 */
				break;
			}
			
			while( flag != true ){
				
				random = ( 1 + rand() % SIZE );		/* 1～9までの数値をランダムに取得 */
				flag = true;
				
				/* 取得した数値がすでに出た数値でないか判定(すでに出た数値であった場合は数値を再取得）*/
				for( k = 0; k < SIZE; k++ ){
					if( num[i][k] == random ){
						flag = false;
						break;
					}
				}
				
				/* 数値をnum配列に格納 */
				if( flag == true ){
					num[i][j] = random;
				}
				
			}
			
			/* 確率分だけ爆弾を配置 */
			if( random <= bombProb )
			{
				bombs[i][j] = 1;
			}
				
		}
		
	}
	return;
}

/*****************************************************************************
* 関数名：displayBoard
*
* 内容　：引数で受け取った1～9の盤面情報、爆弾の位置、すでにめくられている位置
*		をもとに盤面を表示。
*
* 引数　：int board[SIZE][SIZE] , int bombs[SIZE][SIZE] , int open[SIZE][SIZE]
*
* 戻り値：なし
******************************************************************************/
void displayBoard( int board[SIZE][SIZE], int bombs[SIZE][SIZE], int open[SIZE][SIZE] )
{
	int i, j;		/* カウンタ用変数 */
	
	printf( "\n　　1 2 3 4 5 6 7 8 9　\n" );
	printf( "　+--------------------+\n" );
	
	for( i = 0; i < SIZE; i++ )
	{
		printf( "%d | " , i + 1 );
		
		for( j = 0; j < SIZE; j++ )
		{
			if( open[i][j] )
			{
				if( bombs[i][j] )
				{
					printf( "■" );					/* 開けたのが爆弾の場合 */
				}
				else
				{
					printf( "  " );					/* 開けたのが爆弾ではない場合 */
				}
			}
			else
			{
				printf( "%d " , board[i][j] );		/* 未開けの場合 */
			}
		}
		printf( "|\n" );
	}
	printf( "　+--------------------+\n" );
	return;
}

/**************************************************************************************************
* 関数名：chooseCell
*
* 内容　：引数で受け取ったx座標とy座標を1～9の盤面情報、爆弾の位置、すでにめくられている位置
*		と照らし合わせてその結果を戻り値として返す。
*
* 引数　：int board[SIZE][SIZE] , int bombs[SIZE][SIZE] , int open[SIZE][SIZE] , int x , int y
*
* 戻り値：int -1 , 0 , points ( -1：既にめくられている場合  0：爆弾の場合  points：加点 ）
**************************************************************************************************/
int chooseCell( int board[SIZE][SIZE], int bombs[SIZE][SIZE], int open[SIZE][SIZE], int x, int y )
{
	int points; 
	
	/* 開封済みを選択した場合 */
	if( open[y][x] )
    {
        printf( "すでにめくられています。\n" );
        return -1;
    }
	
	open[y][x] = 1;		/* マスをめくる */
	
	/* 爆弾だった場合 */
	if( bombs[y][x] )
    {
        printf( "爆弾！！ -%d点です。\n" , (y + 1) );
        return 0;
    }
	/* セーフ（得点獲得）だった場合 */
	else
    {
        points = board[y][x];
        printf( "%d 点を獲得しました。\n" , points );
        return points;
    }
}

