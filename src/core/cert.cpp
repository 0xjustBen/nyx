#include "core/cert.hpp"

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/rand.h>

#include <cstdio>

namespace nyx {

// Stub generator. Real impl: EC P-256 keypair, X509v3 cert with SubjectAltName
// covering chat.na1.lol.riotgames.com + all regional XMPP hosts + 127.0.0.1,
// 10-year CA, 1-year leaf signed by CA.
CertBundle Cert::generate()
{
    CertBundle b;
    // TODO(phase-1): implement using OpenSSL EVP + X509_* APIs.
    return b;
}

bool Cert::save(const CertBundle & /*b*/, const std::string & /*dir*/)
{
    // TODO: write PEMs to dir with 0600 perms.
    return false;
}

bool Cert::load(CertBundle & /*out*/, const std::string & /*dir*/)
{
    return false;
}

} // namespace nyx
