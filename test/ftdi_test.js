expect = require('chai').expect;
ftdi = require('../index.js');

/*
* FakeFtd2xx is configured to:
* - return 2 devices
* - devices have serialNumber FTDX00 and FTDX01
* - devices have description 'Description for 00.' and 'Description for 01.'
* - vendorIds: 123, 456
* - productIds: 321, 654
*/

describe("ftdi", function(){

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

  });

  describe("list devices by vendorId and productId", function(){
    it("returns the device with matching vendorId", function(done){
        ftdi.find(123, 456, function(err, devices){
          expect(devices.length).to.equal(1);
          expect(devices[0].vendorId).to.equal(123);
          expect(devices[0].productId).to.equal(456);
          done();
      });
    });
  });

  describe("FtdiDevice", function(){

  });
});
