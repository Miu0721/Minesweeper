/*********************************************************************
* 【数取マインスイーパー】メニュー画面
*
* 内容：１～３の整数を受け取り、ゲーム開始、ファイル参照、ゲーム終了を処理
*		ソケット通信は1.ゲーム開始を選択した後に、接続待機状態になる
* 
* 作成日:2024/07/19
* 作成者:秋山 みう
*
*********************************************************************/
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "game.h"
#include "clearBuffer.h"

#define RESULT_FILE "\Result.csv"
#define MENU_NUMBER 10
#define VALIDATION 10
#define RECORD 256

void checkFile();

int main( void )
{
	char menu[MENU_NUMBER]; 			/* メニュー選択時に使う変数 */ 
	int len = 0; 

	
	/* while 1 の間はメニュー機能を回るようにする。*/ 
	while(1)
	{
		/* 表示入力 (メニュー画面) */ 
		printf("メニュー\n"); 
		printf("1 ゲーム開始\n"); 
		printf("2 ゲーム履歴(参照）\n"); 
		printf("3 終了\n"); 
		printf("\n"); 
		printf("メニューを選択してください(半角数字のみで回答）："); 

		fgets(menu, MENU_NUMBER, stdin); 
		
		/* 改行文字を取り除く*/
		len =  strlen( menu );
		if (len > 0 && menu[len - 1] == '\n') {
            menu[len - 1] = '\0';
            len--;
        }
		
		//Enterキーのみ入力された場合 
		if( len == 0 ){
			printf( "入力に誤りがあります。１～３を選択してください。\n");
			
			continue;
		}
		
		/*　一文字以上の文字が入力された場合*/
		if( len > 1 ){
			printf( "入力に誤りがあります。１～３を選択してください。\n");
			
			continue;
		}
		

		switch(*menu){
			case '1': 
				startGame();			/* startGame関数の呼び出し */ 
				continue;
			
			case '2': 
				checkFile(); 
				break;
				
			case '3': 
				return 0; 
			
			default: 
				printf("入力に誤りがあります。１～３を選択してください。\n");
				continue;
		
		}
	}
	
	return 0;
}



/*********************************************************************
* 【数取マインスイーパー】ファイル参照用関数
*
* 関数名：checkFile()
* 内容	：作成されたResult.csvを開き最初に対戦時間を表示、参照したいゲームナンバーを
* 		　選択してプレイヤー名・スコア・勝者を表示する
* 		 続けて参照を続けるか、メニュー画面にもどるかを選択する
*
* 引数　：なし
* 戻り値：なし
*
* 作成日:2024/07/24
* 作成者:秋山 みう
*
*********************************************************************/
void checkFile(){
	
	FILE *fp;								/* ファイルポインタ */
    int i;									/* カウンター */ 
    int count = 0;							/* カウンター */ 
	int current_record = 0;					/* 現在の最大行数格納 */
	char valid_reference_id[VALIDATION]; 	/* 参照するデータIDのバリデーション用 */
	int reference_id;						/* 参照するデータIDの受け取り */
	char history[RECORD];					/* 選択された履歴を格納するための配列 */ 
	char retry[2];							/* 参照を続けるか、続けないか	*/
	char move;								/* 履歴の一覧を出すときに使う変数 */ 
	int pointer;							/* 配列のポインタを動かすときに使う変数 */ 
	bool flag = true;						/* ループフラグ */
	int len; 								/* 入力された文字列の長さを格納 */
	
	// ファイル参照を繰り返すループ：秋山か
	while(flag){
		printf("ゲーム履歴\n"); 
		// ファイルオープン
		if((fp = fopen(RESULT_FILE, "r")) == NULL){
            printf("ファイルが開けませんでした。\n");
			return;
        } 

		/* 履歴が何件あるか数える */
		/* もし、ループしたときは、その都度、current_recordを初期化する。*/ 
		current_record = 0;
		while(1)
		{	
			fgets( history , RECORD , fp );
			if( feof(fp) )
			{
				break;
			}
			current_record++;
		}

		
		/* ポインタをファイルの先頭に戻す */
		fseek( fp , 0 , SEEK_SET );
		fgets( history , RECORD , fp );	// １行目のヘッダをスキップ

		/* 件数分の履歴(日時情報のみ）を表示 */
		for( i = 1; i < current_record; i++ )
		{
			count = 0;
			while( count < 5 )		// 5つのカンマ文スキップして時刻のみ表示
			{
				move = fgetc( fp );
				if( move == ',' )
				{
					count++;
				}
			}
			fgets( history , RECORD , fp );		// 対戦日時を格納
			printf( "%d　%s" , i , history );	// IDと対戦日時を出力
		}

		// ファイルを閉じる
		fclose( fp );
					
		/* 閲覧したい履歴の選択 */
		printf("\n参照するデータを選択してください："); 
		if (fgets(valid_reference_id, VALIDATION, stdin) != NULL) {
			// 改行文字を取り除く
			valid_reference_id[strcspn(valid_reference_id, "\n")] = '\0';

			// 入力が空（Enterキーのみが押された場合）
			if (strlen(valid_reference_id) == 0) {
				printf( "入力に誤りがあります。\n");
				continue;
			}
			/* 英数字以外の入力があった場合 */
			else if( !(0x21 <= valid_reference_id[0] && valid_reference_id[0] <= 0x7e))
			{
				printf( "入力に誤りがあります。\n");
				continue;
			}

			// 数値のバリデーション
			if (sscanf(valid_reference_id, "%d", &reference_id) != 1 || reference_id < 1 || reference_id > (current_record - 1)) {
				printf( "無効な入力です。1から%dの整数を入力してください。\n", (current_record - 1) );
				continue;
			}
		} else {
			// fgetsがNULLを返した場合（エラーが発生した場合）
			printf("入力エラーが発生しました。再入力してください。\n");
			continue;
		}

		// ファイルオープン
		if((fp = fopen(RESULT_FILE, "r")) == NULL){
            printf("ファイルが開けませんでした。\n");
			return;
        } 		

		/* 選択された履歴の文字列まで来た。文字列一行分をすべて、いったん一つの配列にいったん入れる */ 
		for(i = 0; i < reference_id; i++)
		{
			fgets(history, RECORD, fp);
		}
						
		pointer = 0;
					
		/* fgets関数を使い、一文字ずつ取得し、各項目の配列に入れていく。*/ 
		fgets(history, RECORD, fp); 

		/* ファイルを閉じる */ 
		fclose(fp);
				
		printf("Player1\t\t: ");
					
		while (history[pointer] != ',') 
		{
			putchar(history[pointer]);
			pointer++;
		}
			
		pointer++;
					
		/* 「参照する履歴」の表示開始 */ 
		printf("\nスコア\t\t: "); 		
		while (history[pointer] != ',') {
			putchar(history[pointer]);   
			pointer++;
		}
		pointer++; 
					
		printf("\nPlayer2\t\t: "); 	
		while (history[pointer] != ',') {     
			putchar(history[pointer]);
			pointer++;
		}				
		pointer++;  
					
		printf("\nスコア\t\t: "); 
		while (history[pointer] != ','){
			putchar(history[pointer]);	    
			pointer++;
		}	
		pointer++; 
					
		printf("\n勝者\t\t: "); 		
		while (history[pointer] != ','){
			putchar(history[pointer]);    
			pointer++;			
		}		
		pointer++;  
					
		printf("\nゲーム開始時間\t: "); 
		while ( history[pointer] != '\n'){
			putchar(history[pointer]);
			pointer++;	
		}
		
		printf("\n"); 
					
		/* 参照する履歴の表示終了 */ 
		while(1)
		{
			printf("\n参照を続けますか（ｙ/ｎ）：");
			/* Enterキーのみ入力された場合 */
			if((len = scanf( "%[^\n]%*c", retry )) == 0){
				printf( "入力に誤りがあります。半角で入力してください。\n" );
				clearBuf();
				continue;
			}

			// ２文字以上入力があった場合
			if (retry[1] != '\0'){
				printf( "入力に誤りがあります。y か n を入力してください。\n" );
				continue;
			}
			// 入力値がYかyの時
			else if(retry[0] == 'Y' || retry[0] == 'y')
			{
				break;			// 現在のループを抜けて最初のループへ
			}
			// 入力値がNかnの時			
			else if( retry[0] == 'N' || retry[0] == 'n' )
			{
				flag = false; 	// フラグを立てる
				break;			// 現在のループを抜けて処理終了
			}
			// 入力値に誤りがある場合
			else
			{
				printf( "入力に誤りがあります。y か n を入力してください。\n" );
			}
		}
	}
	return;
}