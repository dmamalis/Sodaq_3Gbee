#ifndef _SODAQ_3GBEE_h
#define _SODAQ_3GBEE_h

#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>
#include <Stream.h>
#include "Sodaq_3Gbee_Modem/Sodaq_GSM_Modem.h"

// TODO this needs to be set in the compiler directives. Find something else to do
#define SODAQ_GSM_TERMINATOR CRLF

#define DEFAULT_READ_MS 5000

typedef void (*CallbackMethodPtr)(ResponseTypes& response, const char* buffer, size_t size, void* parameter);

class Sodaq_3Gbee: public Sodaq_GSM_Modem {
public:
    bool isAlive() override;
    bool setAPN(const char* apn) override;
    bool setAPNUsername(const char* username) override;
    bool setAPNPassword(const char* password) override;

    bool init(Stream& stream, const char* simPin = NULL, const char* apn = NULL, const char* username = NULL, 
        const char* password = NULL, AuthorizationTypes authorization = AutoDetectAutorization) override;
    bool join(const char* apn = NULL, const char* username = NULL, const char* password = NULL, 
        AuthorizationTypes authorization = AutoDetectAutorization);
    bool disconnect() override;

    NetworkRegistrationStatuses getNetworkStatus() override;
    NetworkTechnologies getNetworkTechnology() override;

    bool getRSSI(char* buffer, size_t size) override;
    bool getBER(char* buffer, size_t size) override;
    bool getOperatorName(char* buffer, size_t size) override;
    bool getMobileDirectoryNumber(char* buffer, size_t size) override;
    bool getIMEI(char* buffer, size_t size) override;
    bool getCCID(char* buffer, size_t size) override;
    bool getIMSI(char* buffer, size_t size) override;
    bool getMEID(char* buffer, size_t size) override;
    SimStatuses getSimStatus() override;

    IP_t getLocalIP() override;
    IP_t getHostIP(const char* host) override;

    int createSocket(Protocols protocol, uint16_t localPort = 0) override;
    bool connectSocket(uint8_t socket, const char* host, uint16_t port) override;
    bool socketSend(uint8_t socket, const char* buffer, size_t size) override;
    size_t socketReceive(uint8_t socket, char* buffer, size_t size) override;
    bool closeSocket(uint8_t socket) override;

    size_t httpRequest(const char* url, const char* buffer, size_t size, HttpRequestTypes requestType = GET, char* responseBuffer = NULL, size_t responseSize = 0) override;

    bool openFtpConnection(const char* server, const char* username, const char* password) override;
    bool closeFtpConnection() override;
    bool openFtpFile(const char* filename, const char* path) override;
    bool ftpSend(const char* buffer) override;
    int ftpReceive(char* buffer, size_t size) override;
    bool closeFtpFile() override;
    
    int getSmsList(const char* statusFilter, int* indexList, size_t size) override;
    bool readSms(int index, char* phoneNumber, char* buffer, size_t size) override;
    bool deleteSms(int index) override;
    bool sendSms(const char* phoneNumber, const char* buffer) override;
protected:
    size_t readResponse(char* buffer, size_t size, ResponseTypes& response, CallbackMethodPtr parserMethod, void* callbackParameter = NULL, uint32_t timeout = DEFAULT_READ_MS);

    template<class T>
    size_t readResponse(char* buffer, size_t size, ResponseTypes& response, 
        void(*parserMethod)(ResponseTypes& response, const char* buffer, size_t size, T* parameter), T* callbackParameter = NULL, uint32_t timeout = DEFAULT_READ_MS)
    {
        return readResponse(buffer, size, response, (CallbackMethodPtr)parserMethod, (void*)callbackParameter, timeout);
    }

    size_t readResponse(char* buffer, size_t size, ResponseTypes& response) override;
private:
    uint16_t _socketPendingBytes[7]; // TODO add getter

    //size_t parseUnsolicitedCodes(char* buffer, size_t size);
    static bool startsWith(const char* pre, const char* str);
    static size_t ipToStirng(IP_t ip, char* buffer, size_t size);
    static bool isValidIPv4(const char* str);
    bool setSimPin(const char* simPin);
    bool isConnected(); // TODO move/refactor into Sodaq_GSM_Modem
    static void _cpinParser(ResponseTypes& response, const char* buffer, size_t size, SimStatuses* simStatusResult);
    static void _udnsrnParser(ResponseTypes& response, const char* buffer, size_t size, IP_t* ipResult);
    static void _upsndParser(ResponseTypes& response, const char* buffer, size_t size, IP_t* ipResult);
    static void _upsndParser(ResponseTypes& response, const char* buffer, size_t size, uint8_t* thirdParam);
    static void _usocrParser(ResponseTypes& response, const char* buffer, size_t size, uint8_t* socket);
    static void _usordParser(ResponseTypes& response, const char* buffer, size_t size, char* resultBuffer);
};

extern Sodaq_3Gbee sodaq_3gbee;

#endif