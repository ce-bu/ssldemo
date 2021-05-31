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

SSL_CTX *setup_server_context()
{
    SSL_CTX *ctx;
    ctx = SSL_CTX_new(SSLv23_method());

    string server_pem = get_ssl_root() / "server01.pem";
    string ca_pem = get_ssl_root() / "cacert.pem";

    SSLFATAL_IF(SSL_CTX_load_verify_locations(ctx, ca_pem.c_str(), NULL) != 1);

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, [](int preverify_ok, X509_STORE_CTX *store) -> int
                       {
                           char buf[256];
                           if (!preverify_ok)
                           {
                               const X509 *cert = X509_STORE_CTX_get_current_cert(store);
                               int depth = X509_STORE_CTX_get_error_depth(store);
                               int err = X509_STORE_CTX_get_error(store);
                               LOG(ERROR) << "Certificate preverification failed err=" << X509_verify_cert_error_string(err);
                               X509_NAME_oneline(X509_get_issuer_name(cert), buf, sizeof(buf));
                               LOG(INFO) << "Issuer:" << buf;
                               X509_NAME_oneline(X509_get_subject_name(cert), buf, sizeof(buf));
                               LOG(INFO) << "Subject:" << buf;
                           }
                           return preverify_ok;
                       });

    SSL_CTX_set_default_passwd_cb(ctx, [](char *buf, int size, int rwflag, void *) -> int
                                  {
                                      strncpy(buf, (char *)("1234"), size);
                                      buf[size - 1] = '\0';
                                      return (strlen(buf));
                                  });

    SSLERR_IF(!SSL_CTX_use_certificate_chain_file(ctx, server_pem.c_str()));

    SSLERR_IF(SSL_CTX_use_PrivateKey_file(ctx, server_pem.c_str(), SSL_FILETYPE_PEM) != 1);

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
        LOG(ERROR) << "SSAL_library_init";
        exit(ERR_SSLINIT);
    }

    SSL_CTX *ctx = setup_server_context();

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
