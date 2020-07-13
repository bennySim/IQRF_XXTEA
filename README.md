# XXTEA encryption algorithm
### for IQRF TR modules TR-52DA and compatibles

First upload code into modules with programmer (e.g. CK-USB-04A).

<br/>
IQRFxxteaExampleTX.c and IQRFxxteaExampleRX.c are examples of usage of the XXTEA algorithm:


* __IQRFxxteaExampleTX.c__ - After pressing the button, the program encrypts the contents of bufferRF and sends the message wirelessly
* __IQRFxxteaExampleRX.c__ - The program waits for the message to be received, then decrypts it and sends it over the serial link to the computer
