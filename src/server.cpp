#include <stdio.h>
#include <string>
#include <thread>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <glog/logging.h>
#include "common.h"

using namespace std;

int main(int argc, char **argv)
{
    FLAGS_logtostderr = 1;
    FLAGS_stderrthreshold = 0;
    google::InitGoogleLogging(argv[0]);

    if (!SSL_library_init())
    {
        LOG(ERROR) << "SSAL_library_init";
        exit(ERR_SSLINIT);
    }

    SSL_CTX *ctx = setup_context("server01.pem");

    BIO *acc;
    acc = BIO_new_accept(PORT);

    SSLFATAL_IF(acc == nullptr);
    SSLFATAL_IF(BIO_do_accept(acc) <= 0);

    while (true)
    {
        SSLFATAL_IF(BIO_do_accept(acc) <= 0);
        BIO *client = BIO_pop(acc);

        SSL *ssl = SSL_new(ctx);
        SSLFATAL_IF(ssl == nullptr);

        SSL_set_bio(ssl, client, client);
        auto t = thread([&]
                        {
                            SSLERR_IF(SSL_accept(ssl) <= 0);
                            LOG(INFO) << "new ssl client";
                            while (true)
                            {
                                char buf[32];
                                int nread = SSL_read(ssl, buf, sizeof(buf) - 1);
                                if (nread <= 0)
                                {
                                    LOG(ERROR) << "BIO_read " << nread;
                                    break;
                                }
                                buf[nread] = 0;
                                LOG(INFO) << buf;
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
                        });
        t.detach();
    }
}
