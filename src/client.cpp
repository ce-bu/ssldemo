#include <stdio.h>
#include <string>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <glog/logging.h>
#include "common.h"

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

    SSL_CTX *ctx = setup_context("client01.pem");

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
