<pre>
  eeeee eeeee eeeee eeee       e  eeeee 
  8   8 8  88 8   8 8          8  8   " 
  8e  8 8   8 8e  8 8eee       8e 8eeee 
  88  8 8   8 88  8 88      e  88    88 
  88  8 8eee8 88ee8 88ee 88 8ee88 8ee88

  eeee eeeee eeeee e  
  8      8   8   8 8  
  8eee   8e  8e  8 8e 
  88     88  88  8 88 
  88     88  88ee8 88 
</pre>

# Prerequisites:

**Make sure you installed the ftdi driver: [ftdi](http://www.ftdichip.com/Drivers/D2XX.htm)**

If you're are using a Linux distribution or Mac OS X you can run the **install.sh** script file to install the ftdi driver...
For Windows the libs are shipped with this module.

# Installation

    npm install ftdi

This assumes you have everything on your system necessary to compile ANY native module for Node.js. This may not be the case, though, so please ensure the following are true for your system before filing an issue about "Does not install". For all operatings systems, please ensure you have Python 2.x installed AND not 3.0, [node-gyp](https://github.com/TooTallNate/node-gyp) (what we use to compile) requires Python 2.x.

### Windows:

Ensure you have Visual Studio 2010 installed. If you have any version OTHER THAN VS 2010, please read this: https://github.com/TooTallNate/node-gyp/issues/44 

### Mac OS X:

Ensure that you have at a minimum the xCode Command Line Tools installed appropriate for your system configuration. If you recently upgrade OS, it probably removed your installation of Command Line Tools, please verify before submitting a ticket.

### Linux:

You know what you need for you system, basically your appropriate analog of build-essential. Keep rocking!

# Usage

## Listing or finding devices

```nodejs
var ftdi = require('ftdi');

ftdi.find(0x27f4, 0x0203, function(err, devices) {}); // returns all ftdi devices with
                                                      // matching vendor and product id
```

## Create an FtdiDevice

```nodejs
var ftdi = require('ftdi');

var device = new ftdi.FtdiDevice({
  locationId: 0,
  serialNumber: 0
});

// or
var device = new ftdi.FtdiDevice(0);  // index in list function
```

## All together

```nodejs
var ftdi = require('ftdi');

ftdi.find(0x27f4, 0x0203, function(err, devices) {
  var device = new ftdi.FtdiDevice(devices[0]);

  device.on('error', function(err) {
  });

  device.open({
    baudrate: 115200,
    databits: 8,
    stopbits: 1,
    parity: 'none'
  },
  function(err) {

    device.on('data', function(data) {

    });

    device.write([0x01, 0x02, 0x03, 0x04, 0x05], function(err) {

    });

  });

});
```

# Release Notes

## v1.0.3

- revert "ready for node >= 0.11.4"

## v1.0.2

- fix allocation/deallocation mismatch

## v1.0.1

- ready for node >= 0.11.4

## v1.0.0

- first release


# License

Copyright (c) 2013 Kaba AG

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
