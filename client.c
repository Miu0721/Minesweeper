/*********************************************************************
* 【数取マインスイーパー】クライアントサイド
*
* 基本設計書第0.04版・詳細設計書第.0.NAME版参照
*
* 通信機能、盤面機能、バッファクリア機能はヘッダファイルにて共通化　
* 
* 作成日:2024/07/17
*
*********************************************************************/

#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "net.h"					// TCP/IP通信のヘッダファイル
#include "board.h"					// 盤面機能のヘッダファイル
#include "clearBuffer.h"			// 通信待機時のバッファクリアorタイムアウト、相手接続遮断時のアラート処理

#define SIZE 9						// 盤面のマス数
#define WINNING_SCORE 10			// 勝利条件スコア
#define LOSING_SCORE -20			// 敗北条件スコア
#define NAME 20						// 名前の文字数
#define FIRST_SECOND 30				// 先攻後攻の表示


int main( void )
{
    int sock;					// クライアント側ソケット
	char client_name[NAME];		// クライアントネーム
	char server_name[NAME];		// サーバーネーム
	int len;					// プレイヤー名入力時「空」判定用変数
	int i;						// カウンタ用変数
	bool name_ok_flg;			//プレイヤー名が半角英数字であるか判定用変数
	char comment[FIRST_SECOND];	// 先攻後攻の表示
	int switch_play_flg = 1;	// スイッチプレイフラグ
	int board[SIZE][SIZE];		// ボード情報
    int bombs[SIZE][SIZE];		// 爆弾情報
	int open[SIZE][SIZE];		// open済みボード情報
	int x, y;					// x軸,y軸
	int client_score;			// サーバー側スコア
	int server_score;			// クライアント側スコア
	int points;					// 獲得したポイント
	bool fin_flg = true;		/* 終了条件格納用変数の宣言と初期化 */
	char replay[2];				// リプレイ選択
	char coordiate[50];			// 座標バリデーション用
	
	
	// ソケット接続
	sock = connectClient();
	if(sock <= 0){
		printf("通信を終了します。");
		return -1;
	}
	
	/* クライアント側のプレイヤー名の入力処理 */
	while(1)
	{
		name_ok_flg = true;									/* name_ok_flg( true:英数字のみの入力(正常) false：エラー ) */
		len = 0;
		printf( "ユーザ名を入力してください：" );
		len = scanf( "%[^\n]%*c", client_name );
		
		/* Enterキーのみ入力された場合 */
		if( len == 0 ){
			printf( "入力に誤りがあります。半角で入力してください。\n" );
			clearBuf();
			continue;
		}
		if( strlen( client_name ) >= NAME ){
			printf( "入力に誤りがあります。19字以内で入力してください。\n" );
			continue;
		}
		/* 一文字ずつ英数字で入力されているか確認 */
		for( i = 0; client_name[i] != '\0'; i++ )
		{
			/* スペースをあけて入力された場合 */
			if( isspace( client_name[i] ) )
			{
				printf( "入力に誤りがあります。スペースは使えません。\n" );
				name_ok_flg = false;
				break;
			}
			/* 英数字以外の入力があった場合 */
			else if( !isalnum( client_name[i] ) )
			{
				printf( "入力に誤りがあります。半角で入力してください。\n" );
				name_ok_flg = false;
				break;
			}
			
		}
		
		/* 英数字のみの入力であった場合ループを抜ける */
		if( name_ok_flg == true )
		{
			break;
		}
	}
	send( sock , client_name , NAME , 0 );

	printf( "\nようこそ！数取りマインスーパーゲームへ！\n" );
	printf( "\n■ルール説明■\n" );
	printf( "数取りマインスーパーは10点先取で勝利となります。\n盤面の座標を指定して数字を獲得しましょう。\n" );
	printf( "ただし、中には爆弾も含まれており、爆弾をひくと指定座標の点数分マイナスになります。\n-20点になると敗北です。\n" );
	printf( "数字が大きいほど爆弾の確率があがりますのでお気をつけください！！\n" );
	
	recv( sock , server_name , NAME , 0 );		// サーバー側名前の受け取り
	printf( "%s と対戦！\n" , server_name );
	
	
	while( switch_play_flg != -1 )
	{ 
		server_score = 0;		// サーバー側スコア
		client_score = 0;		// クライアント側スコア

		recv( sock , comment , FIRST_SECOND , 0 );						// 先攻後攻の表示
		printf( "%s" , comment );
		recv( sock , board , sizeof(board) , 0 );				// 盤面情報
		recv( sock , bombs , sizeof(bombs) , 0 );				// 爆弾情報
		recv( sock , open , sizeof(open) , 0 );					// 開示済み盤面情報
		recv( sock , &switch_play_flg , sizeof(switch_play_flg) , 0 );	// ターンの切り替え
		recv( sock , &server_score , sizeof(server_score) , 0 );	// 相手の現在得点
		
		// ゲームループ開始(先攻か後攻か)
		while( switch_play_flg == 0 || switch_play_flg == 1 )
		{
			clearBuffer(sock);					// バッファクリア

			// 先攻後攻確定処理
			if( switch_play_flg == 1 )				// 先攻後攻の結果と盤面を受け取る
			{
				recv( sock , comment , FIRST_SECOND , 0 );
				printf( "%s" , comment );
				recv( sock , board , sizeof(board) , 0 );				// 盤面情報
				recv( sock , bombs , sizeof(bombs) , 0 );				// 爆弾情報
				recv( sock , open , sizeof(open) , 0 );					// 開示済み盤面情報
				recv( sock , &switch_play_flg , sizeof(switch_play_flg) , 0 );	// 継続フラグの受け取り
				recv( sock , &server_score , sizeof(server_score) , 0 );	// 相手の現在得点
			}

			// 座標選択
			// x座標・y座標が正しく入力されるまでのループ
			while(1){
				displayBoard( board , bombs , open );// ボード情報の呼び出し
				printf( "\n相手の得点は現在、%d点です！\n" , server_score );
				printf( "%s、x座標を選んでください（1-9)：" , client_name );
				if (fgets(coordiate, sizeof(coordiate), stdin) != NULL) {
					// 改行文字を取り除く
					coordiate[strcspn(coordiate, "\n")] = '\0';

					// 入力が空（Enterキーのみが押された場合）
					if (strlen(coordiate) == 0) {
						printf( "無効な入力です。1から9の整数を入力してください。\n" );
						continue;
					}
					/* 英数字以外の入力があった場合 */
					else if( !(0x21 <= coordiate[0] && coordiate[0] <= 0x7e))
					{
						printf( "無効な入力です。1から9の整数を入力してください。\n" );
						continue;
					}

					// 数値のバリデーション
					if (sscanf(coordiate, "%d", &x) != 1 || x < 1 || x > 9) {
						printf( "無効な入力です。1から9の整数を入力してください。\n" );
						continue;
					}
				} else {
					// fgetsがNULLを返した場合（エラーが発生した場合）
					printf("入力エラーが発生しました。再入力してください。\n");
					continue;
				}

				// yの入力
				printf( "%s、y座標を選んでください（1-9)：" , client_name );
				if (fgets(coordiate, sizeof(coordiate), stdin) != NULL) {
					// 改行文字を取り除く
					coordiate[strcspn(coordiate, "\n")] = '\0';

					// 入力が空（Enterキーのみが押された場合）
					if (strlen(coordiate) == 0) {
						printf( "無効な入力です。1から9の整数を入力してください。\n" );
						continue;
					}
					/* 英数字以外の入力があった場合 */
					else if( !(0x21 <= coordiate[0] && coordiate[0] <= 0x7e))
					{
						printf( "無効な入力です。1から9の整数を入力してください。\n" );
						continue;
					}

					// 数値のバリデーション
					if (sscanf(coordiate, "%d", &y) != 1 || y < 1 || y > SIZE) {
						printf( "無効な入力です。1から9の整数を入力してください。\n" );
						continue;
					}
				} else {
					// fgetsがNULLを返した場合（エラーが発生した場合）
					printf("入力エラーが発生しました。再入力してください。\n");
					continue;
				}
				// 両方の入力が有効であればループを抜ける
				break;
			}

			// 0-8に合わせるために座標デクリメント
			x--;
			y--;
	
			// 指定された座標を渡し、該当の盤面の結果を返す
			points = chooseCell( board , bombs , open , x , y);
			// 爆弾の場合
			if( points == 0 )
			{
				client_score -= y + 1;
				printf( "%sさんの合計得点は、%d点です。\n\n相手のターンです\n" , client_name , client_score );
			}
			// 得点の場合
			else if( points != -1 )
			{
				client_score += points;
				printf( "%sさんの合計得点は、%d点です。\n\n相手のターンです\n" , client_name , client_score );
			}
			// すでに開示されている場合
			else if( points == -1 )
			{
				continue;
			}
			// 得点が勝敗条件に満たない場合
			if( client_score < WINNING_SCORE && client_score > LOSING_SCORE )
			{
				switch_play_flg = 1;		// 継続のフラグを立てる
			}
			else
			{
				switch_play_flg = -1;										// server側でゲームループを終了させる
				send( sock , board , sizeof(board) , 0 );				// 現在の盤面情報
				send( sock , bombs , sizeof(bombs) , 0 );				// 現在の爆弾情報
				send( sock , open , sizeof(open) , 0 );					// 現在の開示情報
				send( sock , &switch_play_flg , sizeof(switch_play_flg) , 0 );	// 継続フラグ情報
				send( sock , &client_score , sizeof(client_score) , 0 );		// client側の得点情報
				break;													// client側でゲームループを終了する
			}
			
			// ゲーム継続の場合(serverへ情報送信)
			send( sock , board , sizeof(board) , 0 );					// 盤面情報
			send( sock , bombs , sizeof(bombs) , 0 );					// 爆弾情報
			send( sock , open , sizeof(open) , 0 );						// 開示情報
			send( sock , &switch_play_flg , sizeof(switch_play_flg) , 0 );		// 継続フラグ
			send( sock , &client_score , sizeof(client_score) , 0 );			// client側の現在得点
			
			// server側で勝敗条件を満たした場合受け取り情報
			recv( sock , board , sizeof(board) , 0 );					// 盤面情報
			recv( sock , bombs , sizeof(bombs) , 0 );					// 爆弾情報
			recv( sock , open , sizeof(open) , 0 );						// 開示情報
			recv( sock , &switch_play_flg , sizeof(switch_play_flg) , 0 );		// 継続フラグ
			recv( sock , &server_score , sizeof(server_score) , 0 );		// 相手の得点
		}

		// バッファクリア
		clearBuffer(sock);
	
		// 最終盤面結果を表示する
		displayBoard( board , bombs , open );
	
		printf("最終得点:\n");
		printf("%s: %d\n", server_name, server_score);
		printf("%s: %d\n", client_name, client_score);
	
		// 勝敗判定
		// server側の得点が高い場合
		if( server_score > client_score )
    	{
       		printf( "%s の勝ちです！\n" , server_name );
   		}
		// server側の得点が低い場合
   	 	else if( client_score > server_score )
    	{	
       	 	printf( "%s の勝ちです！\n" , client_name );
    	}
   		else
    	{
       		printf( "引き分けです！\n" );
   		}
		printf("%sさんが、ゲームを続けるか決めています。少々お待ちください。\n\n", server_name);
		
		recv( sock , &fin_flg , sizeof(fin_flg) , 0 );		// リプレイ情報の受け取り		

		// 対戦終了機能
		if( fin_flg == true )	// リプレイを希望された場合
		{
			while(1)
			{
				clearBuffer(sock);
				printf( "もう一度プレイしますか？（ y / n )：");
				/* Enterキーのみ入力された場合 */
				if((len = scanf( "%[^\n]%*c", replay )) == 0){
					printf( "入力に誤りがあります。半角で入力してください。\n" );
					clearBuf();
					continue;
				}

				// client側がリプレイを希望しない場合
				if (replay[1] != '\0'){
					printf( "入力に誤りがあります。y か n を入力してください。\n" );
					continue;
				}				
				else if( replay[0] == 'N' || replay[0] == 'n' )
				{
					switch_play_flg = -1;
					send( sock , &switch_play_flg , sizeof(switch_play_flg) , 0 );
					break;
				}
				// client側もリプレイを希望する場合
				else if( replay[0] == 'Y' || replay[0] == 'y' )
				{
					switch_play_flg = 1;
					send( sock , &switch_play_flg , sizeof(switch_play_flg) , 0 );
					break;
				}
				// 入力値に誤りがある場合
				else
				{
					printf( "入力に誤りがあります。y か n を入力してください。\n" );
				}
			}
		}
	}

    // 接続されたソケットを閉じる
    close( sock );
	printf("ゲームを終了します。\n");
    
    return 0;
}



