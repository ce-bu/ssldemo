#include "common.h"
#include <openssl/ssl.h>
#include <glog/logging.h>

std::filesystem::path get_ssl_root()
{
    return fs::canonical(fs::read_symlink("/proc/self/exe").parent_path().parent_path() / "ssl");
}
