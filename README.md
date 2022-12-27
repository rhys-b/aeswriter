# AES Writer

A simple text editor for Linux and Windows that encrypts its contents using the 128-bit Advanced Encryption Standard.

AES Writer uses [wxWidgets](https://wxwidgets.org/) for GUI components, and [LibAES](https://github.com/rhys-b/libaes/) to encrypt the data.

The [provided makefile](https://github.com/rhys-b/aeswriter/makefile/) works but should be used as a template because it is cardcoded to use GCC and G++ as compilers, uses GTK as a graphics toolkit, and assumes the LibAES directory is in the same parent directory that contains the AES Writer directory.
