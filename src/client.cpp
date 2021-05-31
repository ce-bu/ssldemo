#include <stdio.h>
#include <string>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <glog/logging.h>
#include "common.h"

using namespace std;

SSL_CTX *setup_client_context()
{
    SSL_CTX *ctx;
    ctx = SSL_CTX_new(SSLv23_method());

    string client_pem = get_ssl_root() / "client01.pem";

    SSL_CTX_set_default_passwd_cb(ctx, [](char *buf, int size, int rwflag, void *) -> int
                                  {
                                      strncpy(buf, (char *)("1234"), size);
                                      buf[size - 1] = '\0';
                                      return (strlen(buf));
                                  });

    SSLERR_IF(!SSL_CTX_use_certificate_chain_file(ctx, client_pem.c_str()));

    SSLERR_IF(SSL_CTX_use_PrivateKey_file(ctx, client_pem.c_str(), SSL_FILETYPE_PEM) != 1);

    SSL_CTX_set_options(ctx, SSL_OP_ALL | SSL_OP_NO_SSLv2);

    SSLFATAL_IF(SSL_CTX_set_cipher_list(ctx, CIPHER_LIST) != 1);
    return ctx;
}

int main(int argc, char **argv)
{
    FLAGS_logtostderr = 1;
    FLAGS_stderrthreshold = 0;
    google::InitGoogleLogging(argv[0]);

    if (!SSL_library_init())
    {
        LOG(ERROR) << "SSL_library_init";
        exit(ERR_SSLINIT);
    }

    SSL_CTX *ctx = setup_client_context();

    BIO *conn = BIO_new_connect(SERVER ":" PORT);
    SSLFATAL_IF(conn == nullptr);
    SSLFATAL_IF(BIO_do_connect(conn) <= 0);
    LOG(INFO) << "connected";

    SSL *ssl = SSL_new(ctx);
    SSLFATAL_IF(ssl == nullptr);

    SSL_set_bio(ssl, conn, conn);
    SSLFATAL_IF(SSL_connect(ssl) <= 0);

    while (true)
    {
        char buf[80];
        if (!fgets(buf, sizeof(buf), stdin))
            break;
        int total = strlen(buf);
        int num_writen = 0;
        while (num_writen < total)
        {
            int count = SSL_write(ssl, buf + num_writen, total - num_writen);
            if (count <= 0)
            {
                LOG(ERROR) << "BIO_write";
                break;
            }
            num_writen += count;
        }
    }
    if (SSL_get_shutdown(ssl) & SSL_RECEIVED_SHUTDOWN)
    {
        LOG(INFO) << "sucessfull shutdown";
        SSL_shutdown(ssl);
    }
    else
    {
        SSL_clear(ssl);
    }
    SSL_free(ssl);
    BIO_free(conn);
}
