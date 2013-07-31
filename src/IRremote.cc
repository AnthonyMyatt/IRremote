#include <node.h>
#include <v8.h>
#include <cvv8/convert.hpp>
#include <string>
#include <unistd.h>
#include <iostream>

#include <libpwm.h>
#include "SimpleGPIO.h"

#include "IRremoteInt.h"

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
    void enableIROut(int khz);
    void mark(int usec, unsigned int pin);
    void space(int usec, unsigned int pin);

private:
    LibPWM pwm;
    unsigned int selectedPin;
    uv_loop_t *loop;

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
    unsigned int buf[];
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

const unsigned int IRsend::SEND_PIN_1 = 44;	// P8_12 GPIO-44
const unsigned int IRsend::SEND_PIN_2 = 26;	// P8_14 GPIO-26
const unsigned int IRsend::SEND_PIN_3 = 46;	// P8_16 GPIO-46
const unsigned int IRsend::RECV_PIN   = 65;	// P8_18 GPIO-65

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

    target->Set(name, constructor->GetFunction());
}

IRsend::IRsend()
: ObjectWrap()
{
    this->loop = uv_default_loop();

    gpio_export(SEND_PIN_1);
    gpio_export(SEND_PIN_2);
    gpio_export(SEND_PIN_3);
    gpio_export(RECV_PIN);

    gpio_set_dir(SEND_PIN_1, OUTPUT_PIN);
    gpio_set_dir(SEND_PIN_2, OUTPUT_PIN);
    gpio_set_dir(SEND_PIN_3, OUTPUT_PIN);
    gpio_set_dir(RECV_PIN, INPUT_PIN);
}

Handle<Value> IRsend::New(const Arguments& args)
{
    HandleScope scope;

    if (!args.IsConstructCall()) {
        return ThrowException(Exception::TypeError(
                                                   String::New("Use the new operator to create instances of this object"))
                              );
    }

    IRsend* obj = new IRsend();

    obj->Wrap(args.This());

    return args.This();
}

// Private

void IRsend::mark(int time, unsigned int pin)
{
    //std::cout << "MARK" << std::endl;
    gpio_set_value(pin, HIGH);
    if (time > 0) usleep(time);
}

void IRsend::space(int time, unsigned int pin)
{
    //std::cout << "SPACE" << std::endl;
    gpio_set_value(pin, LOW); 
    if (time > 0) usleep(time);
}

void IRsend::enableIROut(int khz)
{
    printf("Enable IR Out: %u kHz\n", khz);

    pwm.stop();

    std::cout << "C1" << std::endl;

    pwm.setPeriod(khz*1000);

    std::cout << "C2" << std::endl;

    pwm.setDuty(0.5);

    std::cout << "C3" << std::endl;

    pwm.run();

    std::cout << "C4" << std::endl;
}

// Public

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

    printf("sendNEC: 0x%lX (%u) - %u\n", data, nbits, pin);

    std::cout << "A1" << std::endl;

    if ( args.Length() > 3 && args[3]->IsFunction() )
    {
        Local<Function> callback = Local<Function>::Cast(args[3]);

	request->callback = Persistent<Function>::New(callback);
    }

    std::cout << "A2" << std::endl;

    uv_work_t req;
    req.data = request;

    std::cout << "A3" << std::endl;

    uv_queue_work(self->loop, &req, sendNEC, doCallback);

    std::cout << "A4" << std::endl;

    return args.This();
}

void IRsend::sendNEC(uv_work_t* req)
{
    std::cout << "Sending NEC Command" << std::endl;

    Generic_req* request = (Generic_req*)req->data;

    std::cout << "B1" << std::endl;

    IRsend* self = request->self;

    std::cout << "B2" << std::endl;

    unsigned long data = request->data;

    std::cout << "B3" << std::endl;

    int nbits = request->nbits;

    std::cout << "B4" << std::endl;

    unsigned int pin = request->pin;

    std::cout << "B5" << std::endl;

    printf("Data: %lX\n", data);
    printf("Len : %u\n", nbits);
    printf("Pin : %u\n", pin);

    self->enableIROut(38);
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

void IRsend::doCallback(uv_work_t* req)
{
   std::cout << "Callback" << std::endl;

    req_data *request = (req_data*)req->data;
    //delete req;

    std::cout << "D1" << std::endl;

    int argc = 0;
    v8::Handle<v8::Value> argv = v8::Null();
    
    std::cout << "D2" << std::endl;

    request->callback->Call(Context::GetCurrent()->Global(), argc, &argv);

    std::cout << "D3" << std::endl;

    request->callback.Dispose();

    //delete request;
}

// Register the module
void RegisterModule(Handle<Object> target) {
    IRsend::Init(target);
}

NODE_MODULE(IRremote, RegisterModule);
