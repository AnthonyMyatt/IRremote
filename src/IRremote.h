//
//  main.h
//  node-IRremote
//
//  Created by Anthony Myatt on 27/06/13.
//  Copyright (c) 2013 Anthony Myatt. All rights reserved.
//

#ifndef node_ir_main_h
#define node_ir_main_h

#include <cstdlib>
//#include <v8.h>
#include <node.h>

//#include <cvv8/convert.hpp>
#include <string>

class decode_results {
public:
    v8::Handle<v8::Value> decode_type;
    v8::Handle<v8::Value> panasonicAddress;
    v8::Handle<v8::Value> value;
    v8::Handle<v8::Value> bits;
    v8::Handle<v8::Value> rawbuf;
    v8::Handle<v8::Value> rawlen;
};

// Values for decode_type
#define NEC 1
#define SONY 2
#define RC5 3
#define RC6 4
#define DISH 5
#define SHARP 6
#define PANASONIC 7
#define JVC 8
#define SANYO 9
#define MITSUBISHI 10
#define UNKNOWN -1

// Decoded value for NEC when a repeat code is received
#define REPEAT 0xffffffff

// main class for receiving IR
class IRrecv : public node::ObjectWrap {
public:
    static v8::Persistent<v8::FunctionTemplate> constructor;
    static void Init(v8::Handle<v8::Object> target);
    IRrecv(int val);
    
protected:
    static v8::Handle<v8::Value> New(const v8::Arguments& args);
    static v8::Handle<v8::Value> blink13(const v8::Arguments& args);
    static v8::Handle<v8::Value>  decode(const v8::Arguments& args);
    static void enableIRIn();
    static void resume();
    
private:
    // These are called by decode
    int getRClevel(decode_results *results, int *offset, int *used, int t1);
    long decodeNEC(decode_results *results);
    long decodeSony(decode_results *results);
    long decodeSanyo(decode_results *results);
    long decodeMitsubishi(decode_results *results);
    long decodeRC5(decode_results *results);
    long decodeRC6(decode_results *results);
    long decodePanasonic(decode_results *results);
    long decodeJVC(decode_results *results);
    long decodeHash(decode_results *results);
    int compare(unsigned int oldval, unsigned int newval);
    
} 
;

// Only used for testing; can remove virtual for shorter code
#ifdef TEST
#define VIRTUAL virtual
#else
#define VIRTUAL
#endif

// main class for sending IR
class IRsend : public node::ObjectWrap {
public:
    static v8::Persistent<v8::FunctionTemplate> constructor;
    static void Init(v8::Handle<v8::Object> target);
    IRsend(int val);
    
protected:
    static v8::Handle<v8::Value> New(const v8::Arguments& args);
    void sendNEC(const v8::Arguments& args);
    void sendSony(const v8::Arguments& args);
    void sendRaw(const v8::Arguments& args);
    void sendRC5(const v8::Arguments& args);
    void sendRC6(const v8::Arguments& args);
    void sendDISH(const v8::Arguments& args);
    void sendSharp(const v8::Arguments& args);
    void sendPanasonic(const v8::Arguments& args);
    void sendJVC(const v8::Arguments& args);
    
    // private
    void enableIROut(int khz);
    VIRTUAL void mark(int usec);
    VIRTUAL void space(int usec);

    unsigned int selectedPin;
}
;

// some useful constants

#define RAWBUF 100  // Length of raw duration buffer

// Marks tend to be 100us too long, and spaces 100us too short when received due to sensor lag.
#define MARK_EXCESS 100

#endif
