iMX28 ADC service plug-in for express-modular-server
===========================================================================

This is a plug-in for [express-modular-server](https://github.com/michael-ts/express-modular-server/).  It provides a service for reading the iMX28 CPU Analog to Digital Convertor values.

If a string option is presented upon initialization, it is the base endpoint to serve GPIO control from.  If this option is not present,  a base endpoint of `/gpio/` is used.

# Install

    npm install service-mx28adc

The device you are running on must have an iMX28 CPU.  This code has been tested on a embeddedTS [TS-7680](https://wiki.embeddedTS.com/wiki/TS-7680) which has a Freescale i.MX286 454MHz ARMv5TE ARM926EJ-S CPU.

Currently only boards with CPU registers at these physical addresses are supported (see TODO below):

    MX LR ADC registers at 0x80050000
    MX HS ADC registers at 0x80002000
    MX CLK CTRL registers at 0x8004000

# Usage

The following example loads the `mx28adc` module with the default endpoint:

    var server = require("express-modular-server")({
         http:true
       })
        // other API calls here
        .API("mx28adc")
        .start()

In this example, an endpoint of `/mx28adc/` is used to control GPIOs:


    var server = require("express-modular-server")({
         http:true
       })
        // other API calls here
        .API("gpio","/mx28adc/")
        .start()

To get the current state of the 8 ADCs on the board, issue an HTTP GET request to the server.  The server will respond with each sample as a raw 16-bit integer followed by a newline.  For example:

    wget http://192.168.1.100/adc
    =>
    1016
    123
    1500
    5
    6950
    1112
    12
    68


# To Do

Allow the memory base address to be configurable so as to support more boards.

# Copyright

Written by Michael Schmidt.

# License

GPL 3.0
