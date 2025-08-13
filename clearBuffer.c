#include "clearBuffer.h"


/*********************************************************************
* 関数名：clearBuffer
*
* 内容：    select関数を使用して通信待機中に標準入力とソケットを監視
*           誤入力があればすべてバッファクリア
*           誤入力がなければタイムアウト
*           相手から接続が切られたらプログラムを終了する
*
* 作成日：2024/07/18
* 作成者：秋山克成
*
*********************************************************************/
void clearBuffer(int sock){
    fd_set readfds;                 // 読み取りFD
    struct timeval timeout;         // タイムアウト
    int maxfd = (sock > STDIN_FILENO) ? sock : STDIN_FILENO;    // 監視FDの最大値を初期化
    int activity;                   // 監視FDの挙動
    char buffer[1024];              // ソケットのバッファ格納用
    ssize_t bytes_read;             // マイナスも格納できる用にssize_t、read関数の戻り値用

    FD_ZERO(&readfds);              // 読み取りFDの初期化
    FD_SET(STDIN_FILENO, &readfds); // 標準入力FDのセット
    FD_SET(sock, &readfds);         // ソケットFDのセット

    timeout.tv_sec = TIMEOUT_SEC;            	// タイムアウト時間設定：秒単位
    timeout.tv_usec = TIMEOUT_MIC;       		// タイムアウト時間設定：マイクロ秒単位(0.1秒でセット)

    // select関数に(最大FD+1、読み取りFD、タイムアウト)を引き渡し、結果をactivityに代入
    activity = select(maxfd + 1, &readfds, NULL, NULL, &timeout);

    // select関数の戻り値が-1で、エラーがEINTRではないとき
    if(activity < 0 && errno != EINTR){
        perror("select error");
    }

    // 待機時に誤入力があった場合の処理
    if(activity > 0 && FD_ISSET(STDIN_FILENO, &readfds)){
        while(FD_ISSET(STDIN_FILENO, &readfds)){        // 標準入力が空になるまでバッファクリア
            clearBuf();
            FD_ZERO(&readfds);                          // select関数は１行分しか監視しないので再設定する
            FD_SET(STDIN_FILENO, &readfds);
            FD_SET(sock, &readfds);
            timeout.tv_sec = 0;
            timeout.tv_usec = 100000;
            activity = select(maxfd + 1, &readfds, NULL, NULL, &timeout);
        }
    }
    // ソケット読み取り可だが、読み取り値が-1以下の場合※相手の接続が切れた場合
    if (activity > 0 && FD_ISSET(sock, &readfds)) {
        bytes_read = read(sock, buffer, sizeof(buffer) - 1);        // read関数を使用してsockに含まれるバッファを調べる(終端文字削除)
        if (bytes_read < 0) {           // -1以下ならエラー処理
            perror("read error");
        } else if (bytes_read == 0) {   // 0の場合は相手からのデータ受信なし(接続が切れた)
            printf("接続が閉じられました,ゲームを終了します。\n");
            close(sock);    // ソケットを閉じる
            exit(0);        // 処理を終わらせる
        }
    }
}

/*********************************************************************
* 関数名：clearBuf
*
* 内容：標準入力のバッファをEOFまでクリアする
*
* 作成日：2024/07/18
* 作成者：秋山克成
*
*********************************************************************/
void clearBuf(){
	int clear;
	while ((clear = getchar()) != '\n' && clear != EOF);
}
