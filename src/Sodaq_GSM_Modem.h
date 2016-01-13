#ifndef _SODAQ_GSM_MODEM_h
#define _SODAQ_GSM_MODEM_h

#include <Arduino.h>
#include <stdint.h>
#include <Stream.h>
#include "Sodaq_OnOffBee.h"

#define DEFAULT_TIMEOUT 1000

#define SODAQ_GSM_MODEM_DEFAULT_INPUT_BUFFER_SIZE 128

// TODO handle Watchdog, also use a define to turn handling on/off

// Data authorization type.
enum AuthorizationTypes {
    NoAuthorization = 0,
    PAP = 1,
    CHAP = 2,
    AutoDetectAutorization = 3,
};

// Network registration status.
enum NetworkRegistrationStatuses {
    UnknownNetworkRegistrationStatus = 0,
    Denied,
    NoNetworkRegistrationStatus,
    Home,
    Roaming,
};

// Network Technology.
enum NetworkTechnologies {
    UnknownNetworkTechnology = 0,
    GSM,
    EDGE,
    UTRAN,
    HSDPA,
    HSUPA,
    HSDPAHSUPA,
    LTE,
};

// SIM status.
enum SimStatuses {
    SimStatusUnknown = 0,
    SimMissing,
    SimNeedsPin,
    SimReady,
};

// TCP/UDP Protocol.
enum Protocols {
    TCP = 0,
    UDP,
};

// HTTP request type.
enum HttpRequestTypes {
    POST = 0,
    GET,
    HEAD,
    DELETE,
    PUT,
    HttpRequestTypesMAX = PUT,
};

// FTP mode.
enum FtpModes {
    ActiveMode = 0,
    PassiveMode,
};

// Response type (returned by readResponse() and parser methods).
enum ResponseTypes {
    ResponseNotFound = 0,
    ResponseOK = 1,
    ResponseError = 2,
    ResponsePrompt = 3,
    ResponseTimeout = 4,
    ResponseEmpty = 5,
};

// IP type
typedef uint32_t IP_t;

// callback for changing the baudrate of the modem stream.
typedef void(*BaudRateChangeCallbackPtr)(uint32_t newBaudrate);

#define DEFAULT_READ_MS 5000 // Used in readResponse()

#define NO_IP_ADDRESS ((IP_t)0)

#define IP_FORMAT "%d.%d.%d.%d"

#define IP_TO_TUPLE(x) (uint8_t)(((x) >> 24) & 0xFF), \
                       (uint8_t)(((x) >> 16) & 0xFF), \
                       (uint8_t)(((x) >> 8) & 0xFF), \
                       (uint8_t)(((x) >> 0) & 0xFF)

#define TUPLE_TO_IP(o1, o2, o3, o4) ((((IP_t)o1) << 24) | (((IP_t)o2) << 16) | \
                                     (((IP_t)o3) << 8) | (((IP_t)o4) << 0))

#define SOCKET_FAIL -1

class Sodaq_3GbeeOnOff;

class Sodaq_GSM_Modem {
public:
    // Constructor
    Sodaq_GSM_Modem();

    // Sets the onoff instance
    void setOnOff(Sodaq_OnOffBee & onoff) { _onoff = &onoff; }

    // Turns the modem on and returns true if successful.
    bool on();

    // Turns the modem off and returns true if successful.
    bool off() const;

    // Sets the optional "Diagnostics and Debug" stream.
    void setDiag(Stream& stream) { this->_diagStream = &stream; };

    // Sets the size of the input buffer.
    // Needs to be called before init().
    void setInputBufferSize(size_t value) { this->_inputBufferSize = value; };

    // Returns true if the modem replies to "AT" commands without timing out.
    virtual bool isAlive() = 0;

    // Initializes the modem instance. Sets the modem stream and the on-off power pins.
    virtual void init(Stream& stream, int8_t vcc33Pin, int8_t onoffPin, int8_t statusPin) = 0;

    // Returns the default baud rate of the modem. 
    // To be used when initializing the modem stream for the first time.
    virtual uint32_t getDefaultBaudrate() = 0;

    // Enables the change of the baud rate to a higher speed when the modem is ready to do so.
    // Needs a callback in the main application to re-initialize the stream.
    void enableBaudrateChange(BaudRateChangeCallbackPtr callback) { _baudRateChangeCallbackPtr = callback; };

    // Sets the apn, apn username and apn password to the modem.
    virtual bool setAPN(const char* apn, const char* username, const char* password) = 0;

    // Turns on and initializes the modem, then connects to the network and activates the data connection.
    virtual bool connect(const char* simPin, const char* apn, const char* username,
                      const char* password, AuthorizationTypes authorization = AutoDetectAutorization) = 0;

    // Disconnects the modem from the network.
    virtual bool disconnect() = 0;

    // Returns true if the modem is connected to the network and has an activated data connection.
    virtual bool isConnected() = 0;

    uint8_t getLastCSQ() const { return _lastCSQ; }

    // Returns the current status of the network.
    virtual NetworkRegistrationStatuses getNetworkStatus() = 0;

    // Returns the network technology the modem is currently registered to.
    virtual NetworkTechnologies getNetworkTechnology() = 0;

    // Gets the Received Signal Strength Indication in dBm and Bit Error Rate.
    // Returns true if successful.
    virtual bool getRSSIAndBER(int8_t* rssi, uint8_t* ber) = 0;

    // Gets the Operator Name.
    // Returns true if successful.
    virtual bool getOperatorName(char* buffer, size_t size) = 0;

    // Gets Mobile Directory Number.
    // Returns true if successful.
    virtual bool getMobileDirectoryNumber(char* buffer, size_t size) = 0;

    // Gets International Mobile Equipment Identity.
    // Should be provided with a buffer of at least 16 bytes.
    // Returns true if successful.
    virtual bool getIMEI(char* buffer, size_t size) = 0;

    // Gets Integrated Circuit Card ID.
    // Should be provided with a buffer of at least 21 bytes.
    // Returns true if successful.
    virtual bool getCCID(char* buffer, size_t size) = 0;

    // Gets the International Mobile Station Identity.
    // Should be provided with a buffer of at least 16 bytes.
    // Returns true if successful.
    virtual bool getIMSI(char* buffer, size_t size) = 0;

    // Returns the current SIM status.
    virtual SimStatuses getSimStatus() = 0;

    // Returns the local IP Address.
    virtual IP_t getLocalIP() = 0;

    // Returns the IP of the given host (nslookup).
    virtual IP_t getHostIP(const char* host) = 0;

    // ==== Sockets

    // Creates a new socket for the given protocol, optionally bound to the given localPort.
    // Returns the index of the socket created or -1 in case of error.
    virtual int createSocket(Protocols protocol, uint16_t localPort = 0) = 0;
    
    // Requests a connection to the given host and port, on the given socket.
    // Returns true if successful.
    virtual bool connectSocket(uint8_t socket, const char* host, uint16_t port) = 0;
    
    // Sends the given buffer through the given socket.
    // Returns true if successful.
    virtual bool socketSend(uint8_t socket, const uint8_t* buffer, size_t size) = 0;
    
    // Reads data from the given socket into the given buffer.
    // Returns the number of bytes written to the buffer.
    virtual size_t socketReceive(uint8_t socket, uint8_t* buffer, size_t size) = 0;
    
    // Closes the given socket.
    // Returns true if successful.
    virtual bool closeSocket(uint8_t socket) = 0;

    // ==== TCP

    // Open a TCP connection
    // This is merely a convenience wrapper which can use socket functions.
    virtual bool openTCP(const char *apn, const char *apnuser, const char *apnpwd,
            const char *server, int port, bool transMode=false) = 0;

    // Close the TCP connection
    // This is merely a convenience wrapper which can use socket functions.
    virtual void closeTCP(bool switchOff=true) = 0;

    // Send data via TCP
    // This is merely a convenience wrapper which can use socket functions.
    virtual bool sendDataTCP(const uint8_t *data, size_t data_len) = 0;

    // Receive data via TCP
    // This is merely a convenience wrapper which can use socket functions.
    virtual bool receiveDataTCP(uint8_t *data, size_t data_len, uint16_t timeout=4000) = 0;

    // ==== HTTP

    // Creates an HTTP request using the (optional) given buffer and 
    // (optionally) returns the received data.
    // endpoint should include the initial "/".
    virtual size_t httpRequest(const char* url, uint16_t port, const char* endpoint,
            HttpRequestTypes requestType = GET,
            char* responseBuffer = NULL, size_t responseSize = 0,
            const char* sendBuffer = NULL, size_t sendSize = 0) = 0;

    // ==== FTP

    // Opens an FTP connection.
    virtual bool openFtpConnection(const char* server, const char* username, const char* password, FtpModes ftpMode) = 0;
    
    // Closes the FTP connection.
    virtual bool closeFtpConnection() = 0;

    // Opens an FTP file for sending or receiving.
    // filename should be limited to 256 characters (excl. null terminator)
    // path should be limited to 512 characters (excl. null temrinator)
    virtual bool openFtpFile(const char* filename, const char* path = NULL) = 0;
    
    // Sends the given "buffer" to the (already) open FTP file.
    // Returns true if successful.
    // Fails immediatelly if there is no open FTP file.
    virtual bool ftpSend(const char* buffer, size_t size) = 0;

    // Fills the given "buffer" from the (already) open FTP file.
    // Returns true if successful.
    // Fails immediatelly if there is no open FTP file.
    virtual int ftpReceive(char* buffer, size_t size) = 0;

    // Closes the open FTP file.
    // Returns true if successful.
    // Fails immediatelly if there is no open FTP file.
    virtual bool closeFtpFile() = 0;

    // ==== SMS
    
    // Gets an SMS list according to the given filter and puts the indexes in the "indexList".
    // Returns the number of indexes written to the list or -1 in case of error.
    virtual int getSmsList(const char* statusFilter = "ALL", int* indexList = NULL, size_t size = 0) = 0;
    
    // Reads an SMS from the given index and writes it to the given buffer.
    // Returns true if successful.
    virtual bool readSms(uint8_t index, char* phoneNumber, char* buffer, size_t size) = 0;
    
    // Deletes the SMS at the given index.
    virtual bool deleteSms(uint8_t index) = 0;

    // Sends a text-mode SMS.
    // Expects a null-terminated buffer.
    // Returns true if successful.
    virtual bool sendSms(const char* phoneNumber, const char* buffer) = 0;

protected:
    // The stream that communicates with the device.
    Stream* _modemStream;

    // The (optional) stream to show debug information.
    Stream* _diagStream;

    // The size of the input buffer. Equals SODAQ_GSM_MODEM_DEFAULT_INPUT_BUFFER_SIZE
    // by default or (optionally) a user-defined value when using USE_DYNAMIC_BUFFER.
    size_t _inputBufferSize;

    // Flag to make sure the buffers are not allocated more than once.
    bool _isBufferInitialized;

    // The buffer used when reading from the modem. The space is allocated during init() via initBuffer().
    char* _inputBuffer;

    // The timeout used by the stream helper methods contained in this class (such as timedRead()).
    uint32_t _timeout;

    // The on-off pin power controller object.
    Sodaq_OnOffBee* _onoff;

    // The callback for requesting baudrate change of the modem stream.
    BaudRateChangeCallbackPtr _baudRateChangeCallbackPtr;

    // This flag keeps track if the next write is the continuation of the current command
    // A Carriage Return will reset this flag.
    bool _appendCommand;

    // This is the value of the most recent CSQ
    uint8_t _lastCSQ;

    // This is the number of second it took when CSQ was record last
    uint8_t _CSQtime;

    // Initializes the input buffer and makes sure it is only initialized once.
    // Safe to call multiple times.
    void initBuffer();

    // Sets the modem stream.
    void setModemStream(Stream& stream);

    // Returns a character from the modem stream if read within _timeout ms or -1 otherwise.
    int timedRead() const;

    // Fills the given "buffer" with characters read from the modem stream up to "length"
    // maximum characters and until the "terminator" character is found or a character read
    // times out (whichever happens first).
    // The buffer does not contain the "terminator" character or a null terminator explicitly.
    // Returns the number of characters written to the buffer, not including null terminator.
    size_t readBytesUntil(char terminator, char* buffer, size_t length);

    // Fills the given "buffer" with up to "length" characters read from the modem stream.
    // It stops when a character read timesout or "length" characters have been read.
    // Returns the number of characters written to the buffer.
    size_t readBytes(char* buffer, size_t length);

    // Returns true if the modem is on.
    bool isOn() const;

    // Reads a line (up to but not including the SODAQ_GSM_TERMINATOR) from the modem
    // stream into the "buffer". The buffer is terminated with null.
    // Returns the number of bytes read, not including the null terminator.
    size_t readLn(char* buffer, size_t size, long timeout = DEFAULT_TIMEOUT);

    // Reads a line from the modem stream into the input buffer.
    // Returns the number of bytes read.
    size_t readLn() { return readLn(_inputBuffer, _inputBufferSize); };

    // Methods to write to the modem stream
    size_t write(const char* buffer);
    size_t write(uint8_t value);
    size_t write(uint32_t value);
    size_t write(char value);

    // Methods to write to the modem stream with terminator
    size_t writeLn(const char* buffer);
    size_t writeLn(uint8_t value);
    size_t writeLn(uint32_t value);
    size_t writeLn(char value);

    // Write the command terminator
    size_t writeLn();

    // Write the command prolog (just for debugging
    void writeProlog();

    virtual ResponseTypes readResponse(char* buffer, size_t size, size_t* outSize, uint32_t timeout = DEFAULT_READ_MS) = 0;
};


#endif