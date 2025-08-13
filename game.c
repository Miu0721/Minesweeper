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
*
*********************************************************************/
#include "game.h"
#include "net.h"
#include "board.h"
#include "clearBuffer.h"


/* サーバー側の対戦用関数 */
int startGame( void )
{
	int sock;						/* 接続済みソケット */
	int sock_fd;					/* リスニングソケット */
	int recv_size;					/* recv()中に切断が切れた場合の判定用変数 */
	char client_name[NAME];			/* クライアントのプレイヤー名 */
	char server_name[NAME];			/* サーバーのプレイヤー名 */
	int len;						/* プレイヤー名入力時「空」判定用変数 */
	int i;							/* カウンタ用変数 */
	bool name_ok_flg;				/* プレイヤー名が半角英数字であるか判定用変数 */
	int order;						/* 先攻後攻の結果格納用変数 */
	char comment[FIRST_SECOND];		/* 先攻後攻の表示 */
	char dateTime[20];				/* 日時格納用変数 */
	int board[SIZE][SIZE];			/* 盤面の1～9 */
    int bombs[SIZE][SIZE];			/* 盤面の爆弾 */
	int open[SIZE][SIZE];			/* めくられた盤面 */
	int server_score;				/* サーバ側の得点 */
	int client_score;				/* クライアント側の得点 */
	int x , y;						/* x座標、y座標 */
	int switch_play_flg = 0;		/* ターン格納用変数の宣言と初期化(0：クライアント側のターン　1：サーバ側のターン　-1：終了) */
	int points;						/* chooseCell関数戻り値格納用変数 */
	bool fin_flg = true;			/* 終了条件格納用変数の宣言と初期化 */
	char replay;					/* リプレイするか否か判定用変数 */
	char coordiate[50];				/* 座標バリデーション用 */
	
	
	/* 乱数の初期化 */
	srand( time(NULL) );
	
	/* ソケット通信、戻り値(接続済みソケット)をsockに代入、引数のアドレスにリスニングソケットを格納 */
	sock = connectServer( &sock_fd );
	if( sock == -1 )
	{
		printf( "接続にエラーが発生しました。\n" );
		return -1;
	}
	
	
	/* クライアント側のプレイヤー名を受信 */
	recv_size = recv( sock , client_name , NAME , 0 );
	
	/* クライアントがプレイヤー名入力時に接続を終了した場合 */
	if (recv_size == 0) {
		printf( "接続が閉じられました。ゲームを終了します。\n" );
		/* ソケットを閉じる */
    	close( sock );
		/* リスニングソケットを閉じる */
		close( sock_fd );
		return -1;
	}
		
	printf( "%s と対戦！\n" , client_name );
	
	
	/* サーバ側のプレイヤー名の入力処理 */
	while(1)
	{
		name_ok_flg = true;								/* name_ok( true:英数字のみの入力(正常) false：エラー ) */
		len = 0;										/* 文字数格納用変数の初期化 */
		printf( "ユーザ名を入力してください：" );
		len = scanf( "%[^\n]%*c", server_name );
		
		/* Enterキーのみ入力された場合 */
		if( len == 0 ){
			printf( "入力に誤りがあります。半角で入力してください。\n" );
			getchar();
			continue;
		}
		
		/* 20字以上の入力があった場合 */
		if( strlen( server_name ) >= NAME ){
			printf( "入力に誤りがあります。20文字未満で入力してください。\n" );
			continue;
		}
		
		/* クライアント側のプレイヤー名と同一の場合 */
		if( strcmp( server_name , client_name ) == 0 ){
			printf( "入力に誤りがあります。クライアントと名前が同一です。\n" );
			continue;
		}
		
		/* 一文字ずつ英数字で入力されているか確認 */
		for( i = 0; server_name[i] != '\0'; i++ )
		{
			/* スペースをあけて入力された場合 */
			if( isspace( server_name[i] ) )
			{
				printf( "入力に誤りがあります。スペースは使えません。\n" );
				name_ok_flg = false;
				break;
			}
			/* 英数字以外の入力があった場合 */
			else if( !isalnum( server_name[i] ) )
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
			
	/* サーバー側のプレイヤー名を送信 */	
	send( sock , server_name , NAME , 0 );
	
	printf( "\nようこそ！数取りマインスーパーゲームへ！\n" );
	printf( "\n■ルール説明■\n" );
	printf( "数取りマインスーパーは10点先取で勝利となります。\n盤面の座標を指定して数字を獲得しましょう。\n" );
	printf( "ただし、中には爆弾も含まれており、爆弾をひくと指定座標の点数分マイナスになります。\n-20点になると敗北です。\n" );
	printf( "数字が大きいほど爆弾の確率があがりますのでお気をつけください！！\n" );
	
	/* 対戦開始日時の取得 */
	getTime( dateTime , sizeof( dateTime ) );
	

	/* 終了条件(switch_play_flg == -1)になるまでループ */
	while( switch_play_flg != -1 )
	{
		/* 得点・盤面の初期化 */
		server_score = 0;
		client_score = 0;
		setBoard( board, bombs, open );
		
		/* 先攻後攻結果をorderに代入 */
		order = orderFirst();
		
		/* サーバ側が先攻の場合 */
		if( order == 0 )
		{
			printf( "\n%sさんが、先攻です。\n" , server_name );
			switch_play_flg = 1;
			strcpy( comment, "後攻です。" ); ;
			send( sock , comment , sizeof(comment) , 0 );
		}
		/* サーバ側が後攻の場合 */
		else if( order == 1 )
		{
			printf( "\n%sさんは、後攻です。\n" , server_name );
			switch_play_flg = 0;
			strcpy( comment, "先攻です。" );
			/* 盤面情報・ターン情報・得点を送信 */
			send( sock , comment , FIRST_SECOND , 0 );
			send( sock , board , sizeof(board) , 0 );
			send( sock , bombs , sizeof(bombs) , 0 );
			send( sock , open , sizeof(open) , 0 );
			send( sock , &switch_play_flg , sizeof(switch_play_flg) , 0 );
			send( sock , &server_score , sizeof(server_score) , 0 );
		}
		
		/* サーバとクライアント間でターンが動いている間ループ */
		while( switch_play_flg == 1 || switch_play_flg == 0 )
		{
			/* 相手ターン中の入力バッファを監視 */
			clearBuffer(sock);
			
			/* 1ターン目がクライアント側であった場合、クライアント側の1ターン目終了時点の情報を受信する */
			if( switch_play_flg == 0 )
			{
				recv( sock , board , sizeof(board) , 0 );
				recv( sock , bombs , sizeof(bombs) , 0 );
				recv( sock , open , sizeof(open) , 0 );
				recv( sock , &switch_play_flg , sizeof(switch_play_flg) , 0 );
				recv( sock , &client_score , sizeof(client_score) , 0 );
				clearBuffer(sock);
			}

			
			/* サーバ側のターンの場合 */
			/* 盤面の表示、x座標とy座標の入力処理 */
			while(1)
			{
				/* 盤面とクライアント側の得点を表示 */
				displayBoard( board , bombs , open );
				printf( "相手の得点は現在、%d点です！\n" , client_score );
				
				/* x座標の入力処理 */
				printf( "%s、x座標を選んでください（1-9)：" , server_name );
				if (fgets(coordiate, sizeof(coordiate), stdin) != NULL) 
				{
					coordiate[strcspn(coordiate, "\n")] = '\0';			/* 改行文字を取り除く */

					/* Enterキーのみが押された場合 */
					if (strlen(coordiate) == 0) 
					{
						printf( "無効な入力です。1から9の整数を入力してください。\n" );
						continue;
					}
					/* 英数字以外の入力があった場合 */
					else if( !(0x21 <= coordiate[0] && coordiate[0] <= 0x7e))
					{
						printf( "無効な入力です。1から9の整数を入力してください。\n" );
						continue;
					}

					/* 1～9以外の数値の入力があった場合 */
					if (sscanf(coordiate, "%d", &x) != 1 || x < 1 || x > 9) 
					{
						printf( "無効な入力です。1から9の整数を入力してください。\n" );
						continue;
					}
				}
				else 
				{
					/* fgetsがNULLを返した場合（エラーが発生した場合）*/
					printf("入力エラーが発生しました。再入力してください。\n");
					continue;
				}

				/* y座標の入力処理 */
				printf( "%s、y座標を選んでください（1-9)：" , server_name );
				if (fgets(coordiate, sizeof(coordiate), stdin) != NULL)
				{
					coordiate[strcspn(coordiate, "\n")] = '\0';				/* 改行文字を取り除く */

					/* Enterキーのみが押された場合 */
					if (strlen(coordiate) == 0) 
					{
						printf( "無効な入力です。1から9の整数を入力してください。\n" );
						continue;
					}
					/* 英数字以外の入力があった場合 */
					else if( !(0x21 <= coordiate[0] && coordiate[0] <= 0x7e))
					{
						printf( "無効な入力です。1から9の整数を入力してください。\n" );
						continue;
					}

					/* 1～9以外の数値の入力があった場合 */
					if (sscanf(coordiate, "%d", &y) != 1 || y < 1 || y > 9)
					{
						printf( "無効な入力です。1から9の整数を入力してください。\n" );
						continue;
					}
				}
				else
				{
					/* fgetsがNULLを返した場合（エラーが発生した場合）*/
					printf("入力エラーが発生しました。再入力してください。\n");
					continue;
				}
				
				/* 両方の入力が有効であればループを抜ける */
				break;
			}
			
			/* 配列の要素数は0～8であるため、x座標、y座標それぞれ-1する */
			x--;
			y--;
			
			/* 加点か爆弾か判定(chooseCell)、戻り値をpointsに代入 */
			points = chooseCell( board , bombs , open , x , y );
			
			/* 爆弾の場合 */
			if( points == 0 )
			{
				server_score -= y + 1;
				printf( "%sさんの合計得点は、%d点です。\n" , server_name , server_score );
			}
			/* 加点の場合 */
			else if( points != -1 )
			{
				server_score += points;
				printf( "%sさんの合計得点は、%d点です。\n" , server_name , server_score );
			}
			/* すでにめくられている場合、x,y座標の入力処理に戻る */
			else if( points == -1 )
			{
				continue;
			}
			
			/* 得点が-20より大きく10未満であった場合、クライアント側のターンに(switch_play_flg = 0)する */
			if( server_score < WINNING_SCORE && server_score > LOSING_SCORE )
			{
				switch_play_flg = 0;
				printf( "\n相手のターンです。\n" );
			}
			/* 得点が-20以下または10以上の場合、終了(switch_play_flg = -1)とし最終結果をクライアント側に送信、ループを抜ける */
			else
			{
				switch_play_flg = -1;
				send( sock , board , sizeof(board) , 0 );
				send( sock , bombs , sizeof(bombs) , 0 );
				send( sock , open , sizeof(open) , 0 );
				send( sock , &switch_play_flg , sizeof(switch_play_flg) , 0 );
				send( sock , &server_score , sizeof(server_score) , 0 );
				break;
				
			}
			
			/* 継続の場合は、クライアント側に情報を送信しクライアント側のターン終了時点の情報を受信 */
			send( sock , board , sizeof(board) , 0 );
			send( sock , bombs , sizeof(bombs) , 0 );
			send( sock , open , sizeof(open) , 0 );
			send( sock , &switch_play_flg , sizeof(switch_play_flg) , 0 );
			send( sock , &server_score , sizeof(server_score) , 0 );
			
			recv( sock , board , sizeof(board) , 0 );
			recv( sock , bombs , sizeof(bombs) , 0 );
			recv( sock , open , sizeof(open) , 0 );
			recv( sock , &switch_play_flg , sizeof(switch_play_flg) , 0 );
			recv( sock , &client_score , sizeof(client_score) , 0 );
			
		}
		
		/* 最終結果表示前の入力バッファを監視しバッファをクリアに */
		clearBuffer(sock);
		
		/* 最終結果の盤面を表示 */
		displayBoard( board , bombs , open );
		
		/* 最終結果を結果ファイルに書き込む */
		makeFile(server_name, client_name, server_score, client_score, dateTime);
		
		/* 最終結果を表示 */
		printf("最終得点:\n");
		printf("%s: %d\n", client_name, client_score);
		printf("%s: %d\n", server_name, server_score);
		
		if( client_score > server_score )
    	{
       		printf( "%s の勝ちです！\n" , client_name );
   		}
    	else if( server_score > client_score )
    	{
        	printf( "%s の勝ちです！\n" , server_name );
    	}
   		else
    	{
        	printf( "引き分けです！\n" );
    	}
		
		
		/* リプレイするか否かの処理 */
		while(1)
		{
			clearBuffer(sock);
			printf( "もう一度プレイしますか？（ y / n )：" );
			scanf( "%c" , &replay );
			
			/* Enterキーのみ入力された場合は入力処理に戻る */
			if( replay == '\n' )
			{
				printf( "入力に誤りがあります。y か n を入力してください。\n" );
				continue;
			}
			
			if( !feof(stdin))
			{
				if( getchar() == '\n' )
				{
					/* 終了の場合、クライアント側に終了を送信しループを抜ける */
					if( replay == 'n' || replay == 'N' )
					{
						fin_flg = false;
						send( sock , &fin_flg , sizeof(fin_flg) , 0 );
						break;
					}
					/* リプレイの場合、クライアント側にリプレイを送信し反応を待つ */
					else if( replay == 'y' || replay == 'Y' )
					{
						fin_flg = true;
						send( sock , &fin_flg , sizeof(fin_flg) , 0 );
						printf( "%sさんのリプレイ待ちです。\n" , client_name );
				
						/* クライアント側のリプレイか否かを受取、終了の場合ループを抜ける */
						recv( sock , &switch_play_flg , sizeof(switch_play_flg) , 0 );
						if( switch_play_flg == -1 )
						{
							printf( "リプレイが拒否されました。メニュー画面に戻ります。\n" );
						}
						break;
					}
					else
					{
						printf( "入力に誤りがあります。y か n を入力してください。\n" );
						continue;
					}
				}
			}
			
			printf( "入力に誤りがあります。y か n を入力してください。\n" );
			clearBuf();
		}
	}
	
	/* ソケットを閉じる */
    close( sock );
	/* リスニングソケットを閉じる */
	close( sock_fd );
	    
	return 0;
}



/*********************************************************************
* 【数取マインスイーパー】時刻取得関数
*
* 日時の取得
*
* 関数名：getTime
*
* 引数　：char *buffer , int size
*
* 戻り値：なし
*
* 作成日:2024/07/10
*
*********************************************************************/
void getTime(char *buffer, int size)
{
	time_t t;			/* 日時格納用変数 */
	struct tm *tm;		/* 現地時間に変換後格納用ポインタ */
	
	t = time( NULL );											/* 日時を取得 */
	tm = localtime( &t );										/* 現地時間に変換 */
	strftime( buffer , size , "%Y-%m-%d %H:%M:%S" , tm );		/* フォーマットに変換し格納 */

	return;
}



/*********************************************************************
* 【数取マインスイーパー】先攻後攻決定関数
*
* 先攻後攻をランダムに決定、戻り値としてその結果を返す。
*
* 関数名：orderFirst
*
* 引数　：なし
*
* 戻り値：int 0 , 1
*
* 作成日:2024/07/10
*
*********************************************************************/
int orderFirst(void) {
    
    int first_player;       /* 先攻後攻結果格納用変数 */

	/* 乱数の初期化 */
    srand(time(NULL));

    /* 先攻と後攻をランダムで決める(0：先攻　1：後攻) */
    first_player = rand() % 2;

	/* 結果(0 or 1)を返す */
    return first_player;
}



/*********************************************************************
* 【数取マインスイーパー】csvファイル関数
*
* 基本設計書第0.04版・詳細設計書第.0.50版参照
*
* 関数：void makeFile(char*, char*, int, int, char*);
* 
* 引数： char*      サーバー側プレイヤー名
*        char*      クライアント側プレイヤー名
*        int        サーバー側最終スコア
*        int        クライアント側最終スコア
*        char*      ゲーム時刻
*
* 戻り値：なし
* 
* 作成日:2024/07/12
*
*********************************************************************/
void makeFile(char* server_name, char* client_name, int my_score, int other_score, char* dateTime)
{
    FILE* fp;                   // ファイルポインタ
    FILE* temp_fp;              // 一時ファイルポインタ
    char line[256];             // １行分の文字列
    char draw[6] = {"draw"};    // 勝者判定"引き分け"
    char* winner;               // 勝者判定
    int line_count = 0;         // 行数カウント

    // 1.勝者判定
    if(my_score > other_score){
        winner = server_name;
    } else if(my_score < other_score){
        winner = client_name;
    } else if(my_score == other_score){
        winner = draw;
    }

    // 2.ファイルが存在しない、初回のみの処理
    if((fp = fopen(RESULT_FILE, "r")) == NULL){     // result.csvがまだ存在しないとき
        fp = fopen(RESULT_FILE, "w");               // result.csvを作成して書き込みモード
        if(fp == NULL){
            printf("ファイルの作成に失敗しました\n");
            return;
        }
        // ヘッダーと初回の戦績を書き込み処理終了
        fprintf(fp, "Player_1,Score,Player_2,Score,Winner,Played_Time\n");
        fprintf(fp, "%s,%d,%s,%d,%s,%s\n",server_name, my_score, client_name, other_score, winner, dateTime);
        fclose(fp);
        return;
    }
    // すでにreslt.csvが存在する場合は、現在の行をカウントしメモリ開放(ファイルポインタをリセットするため)

    // 3.現在のファイル行数をカウントする処理
    while (fgets(line, sizeof(line), fp) != NULL)
    {
        line_count++;
    }
    fclose(fp);

    // 4.再度ファイルを開く
    if((fp = fopen(RESULT_FILE, "r")) == NULL){
        printf("ファイルが開けませんでした\n");
        return;
    }
    
    // 5.一時ファイルを開く
    temp_fp = fopen(TEMP_FILE, "w");            // tempファイルに新たに書き込む
    if(temp_fp == NULL){
        printf("ファイルの更新に失敗しました。\n");
        fclose(fp);
        return;
    }

    // 6.ヘッダーを書き込む(１行目)
    fgets(line, sizeof(line), fp);
    fputs(line, temp_fp);

    // 7.既存データを一時ファイルにコピー
    while (fgets(line, sizeof(line), fp) != NULL)
    {
        // result.csvが11行ある＝レコードが最大の場合処理
        if(line_count > MAX_RECORDS){
            fgets(line, sizeof(line), fp);  // 古いレコード(2行目)を読み飛ばす
            line_count = 0;                 // 行カウントをリセットして,以降ループ内でこの処理を行わない
        }
        fputs(line, temp_fp);               // 3行目以降のレコードを一時ファイルに書き込む
    }
    
    // 8.新しいデータを書き込む(11行目)
    fprintf(temp_fp, "%s,%d,%s,%d,%s,%s\n",server_name, my_score, client_name, other_score, winner, dateTime);
    
    // 9.ファイルを閉じ
    fclose(temp_fp);
	fclose(fp);
    // 元ファイルの削除
    remove(RESULT_FILE);
    // 一時ファイルのリネーム
    rename(TEMP_FILE, RESULT_FILE);  // temp_result.csvがresult.csvにリネームされるので画面上はファイルが更新されたように見える
	
    return;
}

