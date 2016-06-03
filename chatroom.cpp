//
// Created by Frank on 5/29/16.
//

#include "chatroom.h"
#include <sys/errno.h>


#define HEART_BREAK_INTERLEAVE (15)


CConnection::CConnection(char *peer_addr, int peer_port)
{
    struct sockaddr_in sin_addr;
    socklen_t addr_len;

    conn = socket(AF_INET, SOCK_DGRAM, 0);
    if (conn < 0) {
        perror("socket");
    }

    int on = 1;
    setsockopt(conn, SOL_SOCKET, SO_REUSEADDR, (void *)(&on), sizeof(on));

    strcpy(this->peer_addr, peer_addr);
    this->peer_port = peer_port;

}

CConnection::~CConnection()
{
    close(conn);
    conn = -1;
}

ssize_t CConnection::send(char *buf, size_t len)
{
    struct sockaddr_in AddrIn;
    socklen_t addr_len = sizeof(AddrIn);

    bzero(&AddrIn, sizeof(AddrIn));
    AddrIn.sin_family = AF_INET;
    AddrIn.sin_port = htons(peer_port);
//	AddrIn.sin_addr.s_addr = inet_addr(remoteIP);
    inet_pton(AF_INET, peer_addr, (&AddrIn.sin_addr));

    return sendto(conn,
            buf, len, 0,
                  (struct sockaddr *)(&AddrIn), addr_len);
}

ssize_t CConnection::recv(int timeout, char *buf, size_t len)
{
    /**
     * Set Connection Receive Timeout.
     */
    struct timeval TimeVal;
    TimeVal.tv_sec = timeout;
    TimeVal.tv_usec = 0;
    if (0 != setsockopt(conn, SOL_SOCKET, SO_RCVTIMEO, (const char*)(&TimeVal), sizeof(TimeVal))) {
        perror("setsocketopt");
    }

    struct sockaddr_in AddrIn;
    socklen_t addr_len = sizeof(AddrIn);
    ssize_t recvn = 0;
    char remote_addr[64];

    /**
     * Receive the Diagram.
     */
    recvn = recvfrom(conn,
                     buf, len, 0,
                     (struct sockaddr *)(&AddrIn), &addr_len);
    /**
     * Check Timeout
     */
    if (recvn < 0) {
        if (EAGAIN == errno || EINTR == errno) {
            return 0;
        }

        return -1;
    }

    /**
     * 返回对端地址。
     */
    inet_ntop(AF_INET, (&AddrIn.sin_addr), remote_addr, INET_ADDRSTRLEN);
    if (0 == strcmp(remote_addr, peer_addr)) {
        return recvn;
    }

    return 0;
}


/**
 * Background Heartbreak Keep Alive.
 */
void *
CChatRoomUser::heartbreak(void *arg) {

    CChatRoomUser *User = (CChatRoomUser *)(arg);
    ssize_t recvn = 0;
    char recvbuf[1024 * 1024 * 1];
    struct sockaddr_in sin_addr;
    socklen_t addr_len;
    time_t keepalive_utc = 0;


    while (1) {

        /**
         * One Second for Heartbreak Timeout.
         */
        recvn = User->Conn->recv(HEART_BREAK_INTERLEAVE, recvbuf, sizeof(recvbuf));
        if (recvn < 0) {
            /**
             * Error.
             */
            break;

        } else if (0 == recvn) {
            /**
             * Send a Heart
             */
            printf("Send Heartbreak.\r\n");
            if (User->Conn->send("heartbreak", 10) < 0) {
                break;
            }

        } else {

            keepalive_utc = time(NULL);

            /**
             * Receive a Message.
             */
            recvbuf[recvn] = 0;
            if (0 == strcmp(recvbuf, "heartbreak")) {
                continue;
            }

            /**
             * Other Message.
             */
            if (NULL != User->onRecvMesg) {
                printf("Handle Message.\r\n");
                User->onRecvMesg(User->username, User->nickname, recvbuf, recvn);
            }
        }

        if (time(NULL) - keepalive_utc >= 2 * HEART_BREAK_INTERLEAVE) {
            printf("Server Off Line.\r\n");
        }
    }

    pthread_exit(NULL);
}


CChatRoomUser::CChatRoomUser(CConnection *Conn, char *username, char *nickname, char *password)
{
    struct sockaddr_in sin_addr;
    socklen_t addr_len;

    this->Conn = Conn;

    if (0 != pthread_create(&Receiver, NULL, heartbreak, (void *)(this))) {
        perror("pthread_create");
    }

    onRecvMesg = NULL;
}

CChatRoomUser::~CChatRoomUser() {
    delete Conn;
    Conn = NULL;
    pthread_join(Receiver, NULL);
}

CChatRoomUser *CChatRoomUser::Login(char *username, char *passphrase)
{
    /**
     * Create a Connection Hander for Communication.
     */
    CConnection *Conn = new CConnection("127.0.0.1", 12315);

    /**
     * TODO
     * Send Login Diagram to Server.
     * Get the Nickname here.
     */
    char *nickname = "Kitty";

    CChatRoomUser *User = new CChatRoomUser(Conn, username, passphrase, nickname);

    return User;

}