expect = require('chai').expect;
fs = require('fs');

ftdi = require('../index.js');

/*
* FakeFtd2xx is configured to:
* - return 2 devices
* - devices have serialNumber FTDX00 and FTDX01
* - devices have description 'Description for fake device 0.' and 'Description for fake device 1.'
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
        expect(devices[0].description).to.equal("Description for fake device 0.");
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
      try {
        deleteSpyfile(0);
      }
      catch(err) {
        //fail silently if the spyfile does not exist
      }
    });

    describe("open", function(){

      afterEach(function(){
        device.close();
      });

      it("can open a device by index", function(done){
        device = new ftdi.FtdiDevice(0);
        device.open({baudrate: 125000}, function(x,y){
          expect(readSpyFile(0)).to.contain("FT_Open");
          done();
        });
      });

      it("can open a device by serialNumber", function(done){
        device = new ftdi.FtdiDevice({serialNumber: "FTDX00"});
        device.open({baudrate: 125000}, function(x,y){
          expect(readSpyFile(0)).to.contain("FT_Open");
          done();
        });
      });

    });

    describe("close", function(){
      it("calls 'FT_Close' on the device", function(done){
        device = new ftdi.FtdiDevice(0);
        device.open({}, function(){
          device.close(function(){
            expect(readSpyFile(0)).to.contain("FT_Close");
            done();
          });
        });
      });
      it("can handle invalid device indices", function(done){
        // a serialNumber without match in the connected devices
        // will trigger FT_Close
        device = new ftdi.FtdiDevice({serialNumber: "null"});
        device.open({}, function(err, success){
          expect(err).to.equal('FT_DEVICE_NOT_FOUND');
          done();
        });

      });

    });
    describe("incoming data", function(){

      afterEach(function(){
        device.close();
      });

      it("emits a 'data' event", function(done){
        doneCalled = false;
        device = new ftdi.FtdiDevice(0);
        device.open({}, function(){
          device.on('data', function(data){
            expect(data.toString('utf8')).to.equal('Fake Message');
            if(!doneCalled){done();doneCalled=true;}
          });
        });
      });
    });

    describe("write", function(){

      afterEach(function(){
        device.close();
      });

      it("should call FT_Write with the correct data", function(done){
        device = new ftdi.FtdiDevice(0);
        device.open({}, function(){
          device.write('this should have been written', function(){
            expect(readSpyFile(0)).to.contain('FT_Write("this should have been written")');
            done();
          });
        });
      });

    });
  });

});

readSpyFile = function(deviceIndex){
  result = fs.readFileSync('/tmp/node-ftdi_spy'+deviceIndex+'.log', 'utf8');
  return result;
};
deleteSpyfile = function(deviceIndex){
  return fs.unlinkSync('/tmp/node-ftdi_spy'+deviceIndex+'.log');
};