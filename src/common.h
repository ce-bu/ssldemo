#ifndef H_5717B15B_A52F_13B4_C003_B584B643C566
#define H_5717B15B_A52F_13B4_C003_B584B643C566

#include <string>
#include <filesystem>
#include <unistd.h>
#include <openssl/err.h>

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
#define CIPHER_LIST "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH"

std::filesystem::path get_ssl_root();

#endif
