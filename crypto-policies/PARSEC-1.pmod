# Policy DEFAULT:PARSEC-1 dump
#
# Do not parse the contents of this file with automated tools,
# it is provided for review convenience only.
#
# Baseline values for all scopes:
cipher = AES-256-GCM AES-256-CCM CAMELLIA-256-GCM AES-256-CTR CAMELLIA-128-GCM
group = X25519 X448 SECP256R1 SECP384R1 SECP521R1 FFDHE-2048 FFDHE-3072 FFDHE-4096 FFDHE-6144 FFDHE-8192
hash = SHA2-384 SHA2-512 SHA3-384 SHA3-512
key_exchange = RSA DHE DHE-RSA PSK DHE-PSK DHE-GSS
mac = AEAD HMAC-SHA2-256 HMAC-SHA2-384 HMAC-SHA2-512
#-- protocol =
sign = EDDSA-ED25519 EDDSA-ED448 RSA-PSS-SHA2-256 RSA-PSS-SHA2-384 RSA-PSS-SHA2-512 RSA-SHA3-256 RSA-SHA2-256 RSA-SHA3-384 RSA-SHA2-384 RSA-SHA3-512 RSA-SHA2-512 RSA-PSS-SHA2-224 RSA-SHA2-224
arbitrary_dh_groups = 1
min_dh_size = 2048
min_dsa_size = 3072
min_rsa_size = 3072
sha1_in_certs = 0
ssh_certs = 1
ssh_etm = 0
# Scope-specific properties derived for select backends:
cipher@gnutls = AES-256-GCM AES-256-CCM
protocol@gnutls = TLS1.3 TLS1.2 DTLS1.2
cipher@java-tls = AES-256-GCM AES-256-CCM
protocol@java-tls = TLS1.3 TLS1.2 DTLS1.2
protocol@libreswan = IKEv2
cipher@nss = AES-256-GCM AES-256-CCM
protocol@nss = TLS1.3 TLS1.2 DTLS1.2
cipher@openssl = AES-256-GCM AES-256-CCM
protocol@openssl = TLS1.3 TLS1.2 DTLS1.2
