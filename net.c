
#include "net.h"

/*********************************************************************
* 関数名：connectServer
*
* 内容：  サーバー側ソケット通信に関する関数
*
* 作成日:2024/07/10
* 作成者:浦野 稚南美
*
*********************************************************************/
int connectServer( int *sock_fd ) 
{
	int server_fd, new_socket;          /* リスニングソケット、接続済みソケット */
    struct sockaddr_in address;         /* ソケット用アドレス格納変数 */
    int opt = 1;                        /* ソケットオプション有効にするフラグ */
    int addrlen = sizeof(address);      /* bind関数に渡すアドレス構造体のサイズを保持する変数 */
	fd_set readfds;                     /* 読み取り用FD */
	struct timeval timeout;             /* タイムアウト設定用変数 */
	int reaction;                       /* 監視FDの戻り値格納用変数 */
	char clear;                         /* バッファクリア用変数 */
	
    
    /* ソケットファイルディスクリプタを作成する */
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("ソケット作成 失敗");
        return -1;
    }
	
	/* ポート52000にソケットをアタッチ */
    if (setsockopt(server_fd, SOL_SOCKET,
        SO_REUSEADDR | SO_REUSEPORT, &opt,
        sizeof(opt)))
    {
        printf("setsockopt 失敗");
        return -1;
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
	
	/* ポート52000にソケットをバインド */
    if (bind(server_fd, (struct sockaddr *)&address,
        sizeof(address)) < 0)
    {
        printf("bind 失敗");
        return -1;
    }
    
	/* 接続要求を待つ */
	printf("クライアントを待っています。\n");
	if (listen(server_fd, 3) < 0)
    {
        printf("listen 異常");
        return -1;
    }
	
	/* 待機中に入力バッファに動きがないか監視 */
	while(1)
	{
		/* 監視するFDの集合体の初期化・セット */
		FD_ZERO(&readfds);
		FD_SET(STDIN_FILENO , &readfds);
	
		/* タイムアウト時間の設定 */
		timeout.tv_sec = 2;
   	 	timeout.tv_usec = 0;
	
		/* select関数の実行 */
		reaction = select(STDIN_FILENO + 1 , &readfds , NULL , NULL , &timeout );
	
		/* エラー発生の場合 */
		if(reaction == -1){
       		printf("select error");
			break;
    	}
		
		/* タイムアウトの場合 */
		if(reaction == 0){
			break;
		}
	
		/* 監視中に何か入力された場合の処理 */
    	if(reaction > 0 && FD_ISSET(STDIN_FILENO, &readfds)){
       		printf( "接続中です。入力しないでください。\n" );
        	while( ( clear = getchar()) != '\n' && clear != EOF );
    		continue;
    	}
	}
		
	/* 接続受付 */
	new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
	if( new_socket < 0 )
    {
       	printf("accept 失敗");
        return -1;
   	}
			

	/* 引数のアドレスにリスニングソケットを代入 */
	*sock_fd = server_fd;
	
	/* 接続済みソケットを返す */
    return new_socket;
	
}


/*********************************************************************
* 関数名：connectServer
*
* 内容：  クライアント側ソケット通信に関する関数
*
* 作成日:2024/07/10
* 作成者:浦野 稚南美
*
*********************************************************************/
int connectClient( void )
{
    int status, client_fd;				/* 接続要求の戻り値・ソケット */
    struct sockaddr_in serv_addr;		/* ソケット用アドレス格納変数 */

	/* ソケットの作成 */
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n ソケット作成失敗 \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    /* IPv4/IPv6のアドレスをテキストからバイナリに変換 */
    if (inet_pton(AF_INET, MY_SERVER_IP_ADDRESS, &serv_addr.sin_addr) <= 0)
    {
        printf(
            "\n　無効またはサポートされてないアドレスです。 \n");
        return -1;
    }

	/* 接続要求 */
    if ((status = connect(client_fd, (struct sockaddr *)&serv_addr,
                          sizeof(serv_addr))) < 0)
    {
        printf("\n接続失敗\n");
        return -1;
    }

    printf("サーバーに接続しました。\n");
    
	/* 接続済みソケットを返す */
	return client_fd;
}

