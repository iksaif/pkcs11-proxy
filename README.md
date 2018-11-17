PKCS11-Proxy is a proxy for the PKCS11-library.

This project is based on a stripped down Gnome Keyring without all gnome
dependencies and other features.

The proxy tunnels PKCS11-requests over the network.  One possible use
is to store cryptograhic information on a seperate server.  This way
the crypto it can be isolated from the rest of the system.  Beware:
the connection is not encrypted and can easily be sniffed.  You should
use a secure communication-channel, for example stunnel.

Here is an example of using pkcs11-proxy together with SoftHSM (from the
OpenDNSSEC project).  The benefit of this setup is that no extra hardware
is needed at all.  This could also be considered the greatest weakeness.
For demonstration purposes, however, security is not a consideration.

    $ sudo adduser cgielen pkcs11
    $ sudo adduser cgielen softhsm
    
    $ softhsm --init-token --slot 0 --label test
    The SO PIN must have a length between 4 and 255 characters.
    Enter SO PIN:
    The user PIN must have a length between 4 and 255 characters.
    Enter user PIN:
    The token has been initialized.
    
    $ PKCS11_DAEMON_SOCKET="tcp://127.0.0.1:2345" pkcs11-daemon /usr/lib/libsofthsm.so
    $ PKCS11_PROXY_SOCKET="tcp://127.0.0.1:2345" pkcs11-tool --module=/usr/lib/libpkcs11-proxy.so -L Available
    slots: Slot 0           SoftHSM
      token label:   test token manuf:   SoftHSM token model:   SoftHSM
      token flags:   rng, login required, PIN initialized, token initialized,
      other flags=0x40 serial num  :  1
      

Another implementation can be found at https://github.com/hajikhorasani/cryptokimpx
