====================================================
Encryption in Internal Trusted Storage (ITS) Service
====================================================

:Author: TBD
:Organization: TBD
:Contact: TBD

Introduction
============
The internal trusted storage service (ITS) in TF-M is designed to store the most
security-critical assets of an application. The key wallet is usually considered
the top security-critical asset for most of the use cases. By design, the ITS
service provides strict security assurances for both online and offline access
of the data. For online access, the ITS specification enforces that the data
stored in ITS must not be directly readable/writable outside of the PSA Root of
Trust. For offline access, the ITS specification enforces that the data must
be protected from read and modifications by attackers with physical access to
the device. Even though Nordic devices follow the ITS specification requirements
we strongly believe that a defence-in-depth approach is the preferred strategy
for ensuring data security. In our efforts to follow this approach we introduce
the encryption feature of the ITS.

The ITS encryption can be enabled using the Kconfig option TFM_ITS_ENCRYPTED and
is transparent to the user as long as the Master Key Encryption Key (MKEK) is
populated before usage.

Encryption
==========

The ITS encryption scheme is using the ChaChaPoly1305 authenticated encryption
cipher with optional authenticated data. The cipher has hardware acceleration
support for both of the TF-M enabled Nordic devices (nRF9160/nRF5340). The key
size is 256 bits and each ITS file is encrypted with a different key.

The first step of the process is the generation of the encryption key. To ensure
a different key for every ITS file a key derivation function (KDF) based on CMAC
is used with a unique derivation label for each file. The input key of the KDF
is the MKEK, a symmetric key stored in the Key Managment Unit (KMU) of the
Nordic devices. The MKEK is protected by the KMU peripheral and its key material
cannot be read by software, it can only be used by reference.

The key derivation process generates the 256 bit symmetric encryption key which
is used with the ChaChaPoly1305 authenticated encryption cipher. In order to
strengthen our data integrity guarantees, the metadata of the ITS file (creation
flags/size) are used as authenticated data in the encryption process. In our
scheme we use a unique nonce for each encryption process. The nonce is generated
by concatenating a random seed and an increasing counter. The random seed is
generated once in the boot process and it stays the same until reset.

After a successfull encryption the ITS service stores the encrypted data, the
authenticated tag, the nonce and the file metdata in flash. Note that the
encryption key used for each file is erased from the memory as soon as the
encryption process is finished.

A diagram describing the encryption process is shown below.

.. image:: media/eits_encryption.pdf


Decryption
==========

The decryption of the ITS files shares the same derivation process as the
encryption process. The encrypted data, the unique file nonce, the
authentication tag and the file metadata are retrieved from the flash storage
to become input for the ChachaPoly1305 decryption. Note that since the file
metadata are used as additional authenticated data their integrity is verified
during the decryption process.

A diagram describing the decryption process is shown below.

.. image:: media/eits_decryption.pdf
