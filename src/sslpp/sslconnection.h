/*
 * Copyright 2002 Lite Speed Technologies Inc, All Rights Reserved.
 * LITE SPEED PROPRIETARY/CONFIDENTIAL.
 */

#ifndef SSLCONNECTION_H
#define SSLCONNECTION_H
#include <lsdef.h>
#include <sslpp/hiocrypto.h>

typedef struct bio_st  BIO;
typedef struct x509_st X509;
typedef struct ssl_cipher_st SSL_CIPHER;
typedef struct ssl_st SSL;
typedef struct ssl_ctx_st SSL_CTX;
typedef struct ssl_session_st SSL_SESSION;

class SslClientSessCache;
class SslConnection : public HioCrypto
{
public:
    enum
    {
        DISCONNECTED,
        CONNECTING,
        ACCEPTING,
        WAITINGCERT,
        GOTCERT,
        CONNECTED,
        SHUTDOWN
    };
    enum
    {
        READ = 1,
        WRITE = 2,
        LAST_READ = 4,
        LAST_WRITE = 8
    };

    char wantRead() const   {   return m_iWant & READ;  }
    char wantWrite() const  {   return m_iWant & WRITE; }
    char lastRead() const   {   return m_iWant & LAST_READ; }
    char lastWrite() const  {   return m_iWant & LAST_WRITE; }

    char getFlag() const    {   return m_iFlag;     }
    void setFlag(int flag) {   m_iFlag = flag;     }

    SslConnection();
    ~SslConnection();

    void setSSL(SSL *ssl);
    SSL *getSSL() const    {   return m_ssl;   }

    void release();
    int setfd(int fd);
    int setfd(int rfd, int wfd);

    void toAccept();
    int accept();
    int connect();
    int read(char *pBuf, int len);
    //int pending()           {   return SSL_pending(m_ssl);    }
    int wpending();
    int write(const char *pBuf, int len);
    int writev(const struct iovec *vect, int count, int *finished);
    int flush();
    int shutdown(int bidirectional);
    int checkError(int ret);
    bool isConnected()      {   return m_iStatus == CONNECTED;  }
    int tryagain();

    char getStatus() const   {   return m_iStatus;   }

    X509 *getPeerCertificate() const;
    long getVerifyResult() const;
    int  getVerifyMode() const;
    int  isVerifyOk() const;
    int  buildVerifyErrorString(char *pBuf, int len) const;

    virtual int getEnv(HioCrypto::ENV id, char *&val,int maxValLen);

    const char *getCipherName() const;

    const SSL_CIPHER *getCurrentCipher() const;

    SSL_SESSION *getSession() const;
    int setSession(SSL_SESSION *session) const;
    int isSessionReused() const;
    void setClientSessCache(SslClientSessCache *cache)
    {   m_pSessCache = cache;     }
    int cacheClientSession(SSL_SESSION* session, const char *pHost, int iHostLen);
    void tryReuseCachedSession(const char *pHost, int iHostLen);

    const char *getVersion() const;

    int setTlsExtHostName(const char *pName);

    const char *getTlsExtHostName();

    int getSpdyVersion();

    int updateOnGotCert();

    static void initConnIdx();
    static int getConnIdx()         {   return s_iConnIdx;   }

    static int getCipherBits(const SSL_CIPHER *pCipher, int *algkeysize);
    static int isClientVerifyOptional(int i);

    void  enableRbio()      {   m_iUseRbio = 1;     }
    char *getRawBuffer(int *len);

private:
    int     installRbio(int rfd, int wfd);
    int     readRbioClientHello();
    void    restoreRbio();

    SSL    *m_ssl;
    SslClientSessCache *m_pSessCache;
    char    m_iStatus;
    char    m_iWant;
    char    m_iFlag;
    char    m_iUseRbio;
    static int32_t s_iConnIdx;

    BIO    *m_saved_rbio;
    char   *m_rbioBuf;
    int     m_iRFd;
    int     m_rbioBuffered;

    LS_NO_COPY_ASSIGN(SslConnection);
};

#endif
