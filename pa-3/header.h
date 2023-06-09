#ifndef HEADER_H
#define HEADER_H

#define BUFFER_SIZE 1024

struct ClientMessageHeader
{
    int length;
    char data[BUFFER_SIZE];
};

struct ServerMessageHeader {
    unsigned char msgFlag;
    unsigned char wordLength;
    unsigned char numIncorrect;
    char data[BUFFER_SIZE];
};


#endif