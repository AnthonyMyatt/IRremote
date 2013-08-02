#include <node.h>
#include <v8.h>
#include <cvv8/convert.hpp>
#include <string>
#include <unistd.h>
#include <iostream>
#include <vector>

#include <libpwm.h>
#include "Beagle_GPIO.h"
//#include "SimpleGPIO.h"

#include "IRremoteInt.hh"

//#define DEBUG

#include "debug.h"

using namespace v8;
using namespace std;

class IRsend : public node::ObjectWrap {
public:
    static v8::Persistent<v8::FunctionTemplate> constructor;
    static void Init(v8::Handle<v8::Object> target);
    IRsend();

    // These functions are bound to Javascript
    static v8::Handle<v8::Value> sendNEC(const v8::Arguments& args);
    static v8::Handle<v8::Value> sendSony(const v8::Arguments& args);
    static v8::Handle<v8::Value> sendRaw(const v8::Arguments& args);
    static v8::Handle<v8::Value> sendRC5(const v8::Arguments& args);
    static v8::Handle<v8::Value> sendRC6(const v8::Arguments& args);
    static v8::Handle<v8::Value> sendDISH(const v8::Arguments& args);
    static v8::Handle<v8::Value> sendSharp(const v8::Arguments& args);
    static v8::Handle<v8::Value> sendPanasonic(const v8::Arguments& args);
    static v8::Handle<v8::Value> sendJVC(const v8::Arguments& args);

protected:
    static v8::Handle<v8::Value> New(const v8::Arguments& args);

    static void sendNEC(uv_work_t* req);
    static void sendSony(uv_work_t* req);
    static void sendRaw(uv_work_t* req);
    static void sendRC5(uv_work_t* req);
    static void sendRC6(uv_work_t* req);
    static void sendDISH(uv_work_t* req);
    static void sendSharp(uv_work_t* req);
    static void sendPanasonic(uv_work_t* req);
    static void sendJVC(uv_work_t* req);

    static void doCallback(uv_work_t* req);

    // private
    bool enableIROut(int khz);
    void mark(int usec, unsigned int pin);
    void space(int usec, unsigned int pin);

private:
    LibPWM      pwm;      // PWM Library
    uv_loop_t   *loop;    // LibUV Loop
    Beagle_GPIO gpio;     // Low-Level (mmap) GPIO Control

    static const unsigned int SEND_PIN_1;
    static const unsigned int SEND_PIN_2;
    static const unsigned int SEND_PIN_3;
    static const unsigned int RECV_PIN;
};

// This defines the base structure passed to all uv_work threads
struct req_data
{
    IRsend* self;
    Persistent<Function> callback;
    unsigned int pin;
};

// These extend the base structure for each function
struct Generic_req : req_data	// NEC, Sony, RC5, RC6, DISH, Sharp
{
    unsigned long data;
    int nbits;
    unsigned int pin;
};

struct Raw_req : req_data
{
    vector<unsigned int>  buf;
    int len;
    int hz;
};

struct Panasonic_req : req_data
{
    unsigned int address;
    unsigned long data;
};

struct JVC_req : req_data
{
    unsigned long data;
    int nbits;
    int repeat;
};

// ---------------------------

/*
const unsigned int IRsend::SEND_PIN_1 = 44;	// P8_12 GPIO-44
const unsigned int IRsend::SEND_PIN_2 = 26;	// P8_14 GPIO-26
const unsigned int IRsend::SEND_PIN_3 = 46;	// P8_16 GPIO-46
const unsigned int IRsend::RECV_PIN   = 65;	// P8_18 GPIO-65
 */

const unsigned int IRsend::SEND_PIN_1 = Beagle_GPIO::P8_12;
const unsigned int IRsend::SEND_PIN_2 = Beagle_GPIO::P8_14;
const unsigned int IRsend::SEND_PIN_3 = Beagle_GPIO::P8_16;
const unsigned int IRsend::RECV_PIN   = Beagle_GPIO::P8_18;

Persistent<FunctionTemplate> IRsend::constructor;

void IRsend::Init(Handle<Object> target) {
    HandleScope scope;

    Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
    Local<String> name = String::NewSymbol("IRsend");

    constructor = Persistent<FunctionTemplate>::New(tpl);
    // ObjectWrap uses the first internal field to store the wrapped pointer
    constructor->InstanceTemplate()->SetInternalFieldCount(1);
    constructor->SetClassName(name);

    // Register the constants
    target->Set(String::NewSymbol("SEND_PIN_1"), cvv8::CastToJS<const unsigned int> (SEND_PIN_1));
    target->Set(String::NewSymbol("SEND_PIN_2"), cvv8::CastToJS<const unsigned int> (SEND_PIN_2));
    target->Set(String::NewSymbol("SEND_PIN_3"), cvv8::CastToJS<const unsigned int> (SEND_PIN_3));
    target->Set(String::NewSymbol("RECV_PIN"), cvv8::CastToJS<const unsigned int> (RECV_PIN));

    // Register the functions
    NODE_SET_PROTOTYPE_METHOD(constructor, "sendNEC", sendNEC);
    NODE_SET_PROTOTYPE_METHOD(constructor, "sendRaw", sendRaw);

    target->Set(name, constructor->GetFunction());
}

IRsend::IRsend()
: ObjectWrap()
{
    this->loop = uv_loop_new();

    this->gpio->configurePin( SEND_PIN_1, Beagle_GPIO::kOUTPUT );
    this->gpio->configurePin( SEND_PIN_2, Beagle_GPIO::kOUTPUT );
    this->gpio->configurePin( SEND_PIN_3, Beagle_GPIO::kOUTPUT );
    this->gpio->configurePin( RECV_PIN,   Beagle_GPIO::kINPUT );
    
    this->gpio->writePin( SEND_PIN_1, 0);
    this->gpio->writePin( SEND_PIN_2, 0);
    this->gpio->writePin( SEND_PIN_3, 0);
    
    /*
    gpio_export(SEND_PIN_1);
    gpio_export(SEND_PIN_2);
    gpio_export(SEND_PIN_3);
    gpio_export(RECV_PIN);

    gpio_set_dir(SEND_PIN_1, OUTPUT_PIN);
    gpio_set_dir(SEND_PIN_2, OUTPUT_PIN);
    gpio_set_dir(SEND_PIN_3, OUTPUT_PIN);
    gpio_set_dir(RECV_PIN, INPUT_PIN);
     */
}

IRsend::~IRsend()
{
    LOG_INFO( "Closing IRsend" );
    this->gpio = NULL;
    this->pwm = NULL;
    this->loop = NULL;
}

Handle<Value> IRsend::New(const Arguments& args)
{
    HandleScope scope;

    if (!args.IsConstructCall()) {
        return ThrowException(Exception::TypeError(
                    String::New("Use the new operator to create instances of this object")));
    }

    IRsend* obj = new IRsend();

    obj->Wrap(args.This());

    return args.This();
}

// Private

void IRsend::mark(int time, unsigned int pin)
{
    //std::cout << "MARK" << std::endl;
    gpio->writePin( pin, 1 );
    //gpio_set_value(pin, HIGH);
    if (time > 0) usleep(time);
}

void IRsend::space(int time, unsigned int pin)
{
    //std::cout << "SPACE" << std::endl;
    gpio->writePin( pin, 0 );
    if (time > 0) usleep(time);
}

bool IRsend::enableIROut(int khz)
{
    LOG_INFO("Enable IR Out: %u kHz\n", khz);

    pwm.stop();
    if (!pwm.setPeriod(khz*1000) || !pwm.setDuty(0.5))
    {
        LOG_ERROR("Failed to start IR output");
        return false;
    }
    pwm.run();
    return true;
}

// Public

/* SEND NEC IR DATA */
Handle<Value> IRsend::sendNEC(const Arguments& args)
{
    HandleScope scope;

    if ( args.Length() < 3 )
    {
        return ThrowException(Exception::TypeError(String::New("Bad argument")));
    }

    IRsend* self = ObjectWrap::Unwrap<IRsend>(args.This());

    unsigned long data = cvv8::CastFromJS<unsigned long> (args[0]);
    int nbits = cvv8::CastFromJS<int> (args[1]);
    unsigned int pin = cvv8::CastFromJS<unsigned int> (args[2]);

    Generic_req* request = new Generic_req;
    request->self = self;
    request->data = data;
    request->nbits = nbits;
    request->pin = pin;

    LOG_INFO("Queuing NEC Command On Pin: %u", pin);

    if ( args.Length() > 3 && args[3]->IsFunction() )
    {
        Local<Function> callback = Local<Function>::Cast(args[3]);

        request->callback = Persistent<Function>::New(callback);
    }

    uv_work_t req;
    req.data = request;

    uv_queue_work(self->loop, &req, sendNEC, doCallback);
    uv_run(self->loop);

    return args.This();
}

void IRsend::sendNEC(uv_work_t* req)
{
    LOG_INFO("Sending NEC Command");

    Generic_req* request = (Generic_req*)req->data;

    IRsend* self = request->self;

    unsigned long data = request->data;
    int nbits = request->nbits;
    unsigned int pin = request->pin;

    if (!self->enableIROut(38))
    {
        return;
    }
    self->mark(NEC_HDR_MARK, pin);
    self->space(NEC_HDR_SPACE, pin);
    for (int i = 0; i < nbits; i++)
    {
        if (data & TOPBIT)
        {
            self->mark(NEC_BIT_MARK, pin);
            self->space(NEC_ONE_SPACE, pin);
        }
        else
        {
            self->mark(NEC_BIT_MARK, pin);
            self->space(NEC_ZERO_SPACE, pin);
        }
        data <<= 1;
    }
    self->mark(NEC_BIT_MARK, pin);
    self->space(0, pin); 
}

/* SEND RAW IR DATA */
Handle<Value> IRsend::sendRaw(const Arguments& args)
{
    HandleScope scope;
    
    if ( args.Length() < 4 )
    {
        return ThrowException(Exception::TypeError(String::New("Bad argument")));
    }
    
    IRsend* self = ObjectWrap::Unwrap<IRsend>(args.This());
    
    vector<unsigned int> buf = cvv8::CastFromJS<vector<unsigned int> > (args[0]);
    int len = cvv8::CastFromJS<int> (args[1]);
    unsigned int freq = cvv8::CastFromJS<unsigned int> (args[2]);
    unsigned int pin = cvv8::CastFromJS<unsigned int> (args[3]);
    
    Raw_req* request = new Raw_req;
    request->self = self;
    request->buf = buf;
    request->len = len;
    request->hz = freq;
    request->pin = pin;
    
    LOG_INFO("Queuing Raw Command On Pin: %u", pin);
    
    if ( args.Length() > 4 && args[4]->IsFunction() )
    {
        Local<Function> callback = Local<Function>::Cast(args[4]);
        
        request->callback = Persistent<Function>::New(callback);
    }
    
    uv_work_t req;
    req.data = request;
    
    uv_queue_work(self->loop, &req, sendRaw, doCallback);
    uv_run(self->loop);

    return args.This();
}

void IRsend::sendRaw(uv_work_t* req)
{
    LOG_INFO("Sending Raw Command");
    
    Raw_req* request = (Raw_req*)req->data;
    
    IRsend* self = request->self;
    
    vector<unsigned int> buf = request->buf;
    int len = request->len;
    unsigned int hz = request->hz;
    unsigned int pin = request->pin;
    
    if (!self->enableIROut(hz))
    {
        return;
    }
    
    for (vector<unsigned int>::size_type i = 0; i != buf.size(); i++)
    {
        if (i & 1)
        {
            self->space(buf[i], pin);
        }
        else
        {
            self->mark(buf[i], pin);
        }
    }
    self->space(0, pin);
}


void IRsend::doCallback(uv_work_t* req)
{
    LOG_INFO("Callback\n");

    req_data *request = (req_data*)req->data;
    //delete req;

    int argc = 0;
    v8::Handle<v8::Value> argv = v8::Null();

    request->callback->Call(Context::GetCurrent()->Global(), argc, &argv);

    request->callback.Dispose();

    //delete request;
}

// Register the module
void RegisterModule(Handle<Object> target) {
    IRsend::Init(target);
}

NODE_MODULE(IRremote, RegisterModule);
