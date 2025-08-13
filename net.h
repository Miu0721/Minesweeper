/*********************************************************************
* 【数取マインスイーパー】ソケット通信に関する関数
*
* サーバーサイド、クライアントサイドのソケット通信を行う処理
*
* 関数名：connectServer()
*         connectClinet()
*
* 引数　：connectServer(int *sock_fd)
*         connectClient(なし)
*
* 戻り値：int 0 , 1
*
* 作成日:2024/07/10
*
*********************************************************************/
#ifndef NET_H
#define NET_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <time.h>

#define PORT (52000)							/* ポート番号 */
#define MY_SERVER_IP_ADDRESS ("127.0.0.1")		/* IPアドレス */


/* プロトタイプ宣言 */
int connectServer( int *sock_fd );		/* サーバ側接続用 */
int connectClient( void );				/* クライアント側接続用 */

#endif


