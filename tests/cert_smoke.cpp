// Cert smoke test: generate CA + leaf, dump to temp dir, verify chain with OpenSSL.
// Build target only when NYX_BUILD_TESTS=ON.

#include "core/cert.hpp"

#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#include <openssl/err.h>

#include <cstdio>
#include <filesystem>

namespace fs = std::filesystem;

static X509 *readPem(const fs::path &p)
{
    FILE *f = std::fopen(p.string().c_str(), "rb");
    if (!f) return nullptr;
    X509 *c = PEM_read_X509(f, nullptr, nullptr, nullptr);
    std::fclose(f);
    return c;
}

int main()
{
    using namespace nyx;

    fs::path dir = fs::temp_directory_path() / "nyx-cert-smoke";
    fs::remove_all(dir);

    CertBundle b = Cert::generate();
    if (b.caPem.empty() || b.leafPem.empty()) {
        std::fprintf(stderr, "FAIL: generate\n");
        return 1;
    }
    if (!Cert::save(b, dir.string())) {
        std::fprintf(stderr, "FAIL: save\n");
        return 1;
    }

    X509 *ca   = readPem(dir / "ca.pem");
    X509 *leaf = readPem(dir / "leaf.pem");
    if (!ca || !leaf) { std::fprintf(stderr, "FAIL: read pem\n"); return 1; }

    X509_STORE *store = X509_STORE_new();
    X509_STORE_add_cert(store, ca);
    X509_STORE_CTX *ctx = X509_STORE_CTX_new();
    X509_STORE_CTX_init(ctx, store, leaf, nullptr);

    int ok = X509_verify_cert(ctx);
    if (ok != 1) {
        int err = X509_STORE_CTX_get_error(ctx);
        std::fprintf(stderr, "FAIL: verify (%s)\n", X509_verify_cert_error_string(err));
        return 1;
    }

    X509_STORE_CTX_free(ctx);
    X509_STORE_free(store);
    X509_free(leaf);
    X509_free(ca);

    std::printf("ok\n");
    return 0;
}
