#include "core/cert.hpp"
#include "core/config.hpp"

#import <Security/Security.h>
#import <Foundation/Foundation.h>

#include <filesystem>

namespace fs = std::filesystem;

namespace nyx {
namespace {

NSString *certDir()
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains(
        NSApplicationSupportDirectory, NSUserDomainMask, YES);
    NSString *base = paths.firstObject ?: NSTemporaryDirectory();
    NSString *dir = [base stringByAppendingPathComponent:@"Nyx"];
    [[NSFileManager defaultManager] createDirectoryAtPath:dir
                              withIntermediateDirectories:YES
                                               attributes:nil error:nil];
    return dir;
}

SecCertificateRef loadCert(NSString *pemPath)
{
    NSData *pem = [NSData dataWithContentsOfFile:pemPath];
    if (!pem) return nullptr;
    // Strip PEM armor to get DER.
    NSString *s = [[NSString alloc] initWithData:pem encoding:NSUTF8StringEncoding];
    NSRange begin = [s rangeOfString:@"-----BEGIN CERTIFICATE-----"];
    NSRange end   = [s rangeOfString:@"-----END CERTIFICATE-----"];
    if (begin.location == NSNotFound || end.location == NSNotFound) return nullptr;
    NSUInteger bodyStart = begin.location + begin.length;
    NSString *body = [s substringWithRange:NSMakeRange(bodyStart, end.location - bodyStart)];
    body = [body stringByReplacingOccurrencesOfString:@"\n" withString:@""];
    body = [body stringByReplacingOccurrencesOfString:@"\r" withString:@""];
    NSData *der = [[NSData alloc] initWithBase64EncodedString:body options:0];
    if (!der) return nullptr;
    return SecCertificateCreateWithData(nullptr, (__bridge CFDataRef)der);
}

} // namespace

bool Cert::installTrust()
{
    NSString *dir = certDir();
    NSString *caPath = [dir stringByAppendingPathComponent:@"ca.pem"];

    if (![[NSFileManager defaultManager] fileExistsAtPath:caPath]) {
        CertBundle b = Cert::generate();
        if (b.caPem.empty()) return false;
        if (!Cert::save(b, dir.UTF8String)) return false;
    }

    SecCertificateRef cert = loadCert(caPath);
    if (!cert) return false;

    // Add to user login keychain.
    NSDictionary *add = @{
        (__bridge id)kSecClass:    (__bridge id)kSecClassCertificate,
        (__bridge id)kSecValueRef: (__bridge id)cert,
    };
    OSStatus addStatus = SecItemAdd((__bridge CFDictionaryRef)add, nullptr);
    if (addStatus != errSecSuccess && addStatus != errSecDuplicateItem) {
        CFRelease(cert);
        return false;
    }

    // User-scope trust: kSecTrustSettingsDomainUser does NOT require admin.
    NSDictionary *trust = @{
        (__bridge id)kSecTrustSettingsResult: @(kSecTrustSettingsResultTrustRoot),
    };
    OSStatus trustStatus = SecTrustSettingsSetTrustSettings(
        cert,
        kSecTrustSettingsDomainUser,
        (__bridge CFArrayRef)@[trust]);

    CFRelease(cert);
    return trustStatus == errSecSuccess;
}

bool Cert::uninstallTrust()
{
    NSString *dir = certDir();
    NSString *caPath = [dir stringByAppendingPathComponent:@"ca.pem"];
    SecCertificateRef cert = loadCert(caPath);
    if (!cert) return false;

    OSStatus rm = SecTrustSettingsRemoveTrustSettings(cert, kSecTrustSettingsDomainUser);

    NSDictionary *del = @{
        (__bridge id)kSecClass:    (__bridge id)kSecClassCertificate,
        (__bridge id)kSecMatchItemList: @[(__bridge id)cert],
    };
    SecItemDelete((__bridge CFDictionaryRef)del);

    CFRelease(cert);
    return rm == errSecSuccess || rm == errSecItemNotFound;
}

} // namespace nyx
