#pragma once
#include <stdint.h>

/* Bit fields */
typedef struct {
    uint8_t version:4u;
    uint8_t ihl:4u;             /* IP header length */
    uint8_t tos;                /* Type of Service */
    uint8_t totalLength[2u];
    uint8_t identification[2u];
    uint8_t flags:3u;
    uint16_t fragmentOffset:13u;
    uint8_t ttl;            /* time to live */
    uint8_t protocol;
    uint8_t headerChecksum[2u];
    uint8_t sourceAddress[4u];
    uint8_t destinationAddress[4u];
    uint8_t optionsAndPadding[4u];
} IPv4Header;

typedef struct {
    uint8_t type;
    uint8_t code;
    uint8_t checksum[2u];
    uint8_t data[4u];
} IcmpPacket;

int8_t Ip_initialize(void);

int8_t Ip_processRequest(uint8_t* requestData);

int8_t Ip_sendIPv4Packet(uint8_t protocol, uint8_t* destinationAddress, uint8_t* payload, uint32_t payloadSize);

int8_t Ip_sendPing(uint8_t* destinationAddress);


void Ip_setIPv4Address(uint8_t* address);
