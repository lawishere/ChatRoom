//
// Created by Frank on 5/29/16.
//

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#ifndef CHATROOM_CCHATROOMUSER_H
#define CHATROOM_CCHATROOMUSER_H

#define CHAT_ACK_TIMEO (2000)
#define CHAT_ROOM_PORT (12315)



class CConnection {

private:
    int conn;
    char peer_addr[64];
    int peer_port;

public:
    CConnection(char *peer_addr, int peer_port);
    virtual ~CConnection();

public:
    /**
     * @brief
     *  Send Buffer with Length to Peer.\n
     */
    ssize_t send(char *buf, size_t len);

    /**
     * @brief
     *  Receive Buffer in Maximal Length from Peer in Timeout.\n
     */
    ssize_t recv(int timeout, char *buf, size_t len);

};


class CChatRoomUser {

private:

    CConnection *Conn;

    /**
     * User Name & Password.
     */
    char username[128], passphrase[128], nickname[128];

    /**
     * Background Heartbreak Keep Alive.
     */
    static void *heartbreak(void *arg);

    /**
     * Heardbreak
     */
    pthread_t Receiver;

    /**
     * Chat Room Server.
     */
    char server_addr[128];

private:
    CChatRoomUser(CConnection *Conn, char *username, char *nickname, char *password);

public:
    virtual ~CChatRoomUser();

public:
    /**
     * @brief
     *  Call back Function for UI Implimetion.
     */
    void (*onRecvMesg)(char *user, char *nickname, char *buf, size_t len);


public:

    /**
     * @brief
     *  Create a User Handler with Login.\n
     */
    static CChatRoomUser *Login(char *username, char *passphrase);
};


class CChatRoomServer {


public:
    virtual ~CChatRoomServer() { }

public:
    CChatRoomServer() { }
};


#endif //CHATROOM_CCHATROOMUSER_H
