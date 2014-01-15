expect = require('chai').expect;
fs = require('fs');

ftdi = require('../index.js');

/*
* FakeFtd2xx is configured to:
* - return 2 devices
* - devices have serialNumber FTDX00 and FTDX01
* - devices have description 'Description for 00.' and 'Description for 01.'
* - vendorIds: 123, 456
* - productIds: 321, 654
*/

describe.only("ftdi", function(){

  describe(".find", function(){

    it("returns 2 devices", function(done){
      ftdi.find(function(err, devices){
        expect(devices.length).to.equal(2);
        done();
      });
    });

    it("reads the serialNumber", function(done){
      ftdi.find(function(err, devices){
        expect(devices[0].serialNumber).to.equal("FTDX00");
        done();
      });
    });

    it("reads the description", function(done){
      ftdi.find(function(err, devices){
        expect(devices[0].description).to.equal("Description for 00.");
        done();
      });
    });


    describe("with vendorId and productId", function(){
      it("returns the device with matching vendorId", function(done){
        ftdi.find(123, 456, function(err, devices){
          expect(devices.length).to.equal(1);
          expect(devices[0].vendorId).to.equal(123);
          expect(devices[0].productId).to.equal(456);
          done();
        });
      });
    });
  });

  describe("FtdiDevice", function(){
    var device;

    afterEach(function(){
      deleteSpyfile("00");
    });

    describe("open", function(){

      afterEach(function(){
        device.close();
      });

      it("can open a device by index", function(done){
        device = new ftdi.FtdiDevice(0);
        device.open({baudrate: 125000}, function(x,y){
          expect(readSpyFile("00")).to.contain("FT_Open");
          done();
        });
      });

      it("can open a device by serialNumber", function(done){
        device = new ftdi.FtdiDevice({serialNumber: "FTDX00"});
        device.open({baudrate: 125000}, function(x,y){
          expect(readSpyFile("00")).to.contain("FT_Open");
          done();
        });
      });

    });

    describe("close", function(){
      it("calls 'FT_Close' on the device", function(done){
        device = new ftdi.FtdiDevice(0);
        device.open({}, function(){
          device.close(function(){
            expect(readSpyFile("00")).to.contain("FT_Close");
            done();
          });
        });
      });
    });
  });

});

readSpyFile = function(deviceIndex){
  return fs.readFileSync('/tmp/ftd2xx.spy_'+deviceIndex, 'utf8');
};
deleteSpyfile = function(deviceIndex){
  return fs.unlinkSync('/tmp/ftd2xx.spy_'+deviceIndex);
};