#ifndef H_5717B15B_A52F_13B4_C003_B584B643C566
#define H_5717B15B_A52F_13B4_C003_B584B643C566

#include <string>
#include <filesystem>
#include <unistd.h>

namespace fs = std::filesystem;

#define ERR_SSLINIT 100

#define SSLERR_IF0(cond, err)                                          \
    do                                                                 \
    {                                                                  \
        if (cond)                                                      \
        {                                                              \
            LOG(ERROR) << #cond;                                       \
            auto cb = [](const char *str, size_t len, void *u) {       \
                LOG(err) << __FILE__ << ":" << __LINE__ << " " << str; \
                return 0;                                              \
            };                                                         \
            ERR_print_errors_cb(cb, nullptr);                          \
        }                                                              \
    } while (0)

#define SSLERR_IF(cond) SSLERR_IF0((cond), ERROR)
#define SSLFATAL_IF(cond) SSLERR_IF0((cond), FATAL)

#define SERVER "localhost"
#define PORT "9000"

static inline std::filesystem::path get_ssl_root()
{
    return fs::canonical(fs::read_symlink("/proc/self/exe").parent_path().parent_path() / "ssl");
}

static SSL_CTX *setup_context(const std::string &pem)
{
    SSL_CTX *ctx;
    ctx = SSL_CTX_new(SSLv23_method());

    SSL_CTX_set_default_passwd_cb(ctx, [](char *buf, int size, int rwflag, void *) -> int
                                  {
                                      strncpy(buf, (char *)("1234"), size);
                                      buf[size - 1] = '\0';
                                      return (strlen(buf));
                                  });

    fs::path cert = get_ssl_root() / pem;
    SSLERR_IF(!SSL_CTX_use_certificate_chain_file(ctx, cert.string().c_str()));

    SSLERR_IF(SSL_CTX_use_PrivateKey_file(ctx, cert.string().c_str(), SSL_FILETYPE_PEM) != 1);
    return ctx;
}

#endif
