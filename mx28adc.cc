#include <nan.h>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


volatile unsigned int *mxlradcregs;
volatile unsigned int *mxhsadcregs;
volatile unsigned int *mxclkctrlregs;

void init() {
  int devmem;
  devmem = open("/dev/mem", O_RDWR|O_SYNC);
  assert(devmem != -1);
  
  // LRADC
  mxlradcregs = (unsigned int *) mmap(0, getpagesize(),
				      PROT_READ | PROT_WRITE,
				      MAP_SHARED, devmem, 0x80050000);
  mxhsadcregs = (unsigned int *)mmap(0, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED,
		     devmem, 0x80002000);
  mxclkctrlregs = (unsigned int *)mmap(0, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED,
		       devmem, 0x80040000);

}

void adcread(unsigned long long chan[8]) {
  unsigned int i, x;
  for (i=0;i<8;i++) chan[i] = 0;

  //signed int bivolt;

  mxlradcregs[0x148/4] = 0xfffffff; //Clear LRADC6:0 assignments
  mxlradcregs[0x144/4] = 0x6543210; //Set LRDAC6:0 to channel 6:0
  mxlradcregs[0x28/4] = 0xff000000; //Set 1.8v range
  for(x = 0; x < 7; x++)
    mxlradcregs[(0x50+(x * 0x10))/4] = 0x0; //Clear LRADCx reg
  
  for(x = 0; x < 10; x++) {
    mxlradcregs[0x18/4] = 0x7f; //Clear interrupt ready
    mxlradcregs[0x4/4] = 0x7f; //Schedule conversaion of chan 6:0
    while(!((mxlradcregs[0x10/4] & 0x7f) == 0x7f)); //Wait
    for(i = 0; i < 7; i++)
      chan[i] += (mxlradcregs[(0x50+(i * 0x10))/4] & 0xffff);
  }

  // HDADC
  //Lets see if we need to bring the HSADC out of reset
  if(mxhsadcregs[0x0/4] & 0xC0000000) {
    mxclkctrlregs[0x154/4] = 0x70000000;
    mxclkctrlregs[0x1c8/4] = 0x8000;
    //ENGR116296 errata workaround
    mxhsadcregs[0x8/4] = 0x80000000;
    mxhsadcregs[0x0/4] = ((mxhsadcregs[0x0/4] | 0x80000000) & (~0x40000000));
    mxhsadcregs[0x4/4] = 0x40000000;
    mxhsadcregs[0x8/4] = 0x40000000;
    mxhsadcregs[0x4/4] = 0x40000000;
    
    usleep(10);
    mxhsadcregs[0x8/4] = 0xc0000000;
  }
  
  mxhsadcregs[0x28/4] = 0x2000; //Clear powerdown
  mxhsadcregs[0x24/4] = 0x31; //Set precharge and SH bypass
  mxhsadcregs[0x30/4] = 0xa; //Set sample num
  mxhsadcregs[0x40/4] = 0x1; //Set seq num
  mxhsadcregs[0x4/4] = 0x40000; //12bit mode
  
  while(!(mxhsadcregs[0x10/4] & 0x20)) {
    mxhsadcregs[0x50/4]; //Empty FIFO
  }
  
  mxhsadcregs[0x50/4]; //An extra read is necessary

  mxhsadcregs[0x14/4] = 0xfc000000; //Clr interrupts
  mxhsadcregs[0x4/4] = 0x1; //Set HS_RUN
  usleep(10);
  
  mxhsadcregs[0x4/4] = 0x08000000; //Start conversion
  while(!(mxhsadcregs[0x10/4] & 0x1)) ; //Wait for interrupt

  for(i = 0; i < 5; i++) {
    x = mxhsadcregs[0x50/4];
    chan[7] += ((x & 0xfff) + ((x >> 16) & 0xfff));
  }
  
  
}

static int initted = 0;

void ADC(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  unsigned long long chan[8];
  if (!initted) {
    init();
    initted = 1;
  }
  adcread(chan);
  v8::Local<v8::Array> sample = v8::Array::New(8);
  int i;
  for (i=0;i<8;i++) {
    Nan::Set(sample,i,Nan::New<v8::Number>((int)chan[i]));
  }
  info.GetReturnValue().Set(num);
}

void Init(v8::Local<v8::Object> exports) {
  exports->Set(Nan::New("get").ToLocalChecked(),
               Nan::New<v8::FunctionTemplate>(ADC)->GetFunction());
}

NODE_MODULE(mx28adc, Init)

