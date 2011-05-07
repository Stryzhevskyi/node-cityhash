// Copyright (c) 2011 Robin Zhong.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <v8.h>
#include <string.h>
#include <sstream>
#include <city.h>
#include <citycrc.h>

#define MAX_64_HASH_LEN 21
#define MAX_128_HASH_LEN MAX_64_HASH_LEN*2

using namespace v8;

Local<String>
stringify_hash(uint64 hash) {
    // hash to char
    char buf[MAX_64_HASH_LEN];
    memset(buf, 0, MAX_64_HASH_LEN);
    snprintf(buf, MAX_64_HASH_LEN, "%llu", hash);

    return String::New(buf);
}

Local<String>
stringify_hash(uint128* hash) {
    // hash to char
    char buf[MAX_128_HASH_LEN];
    memset(buf, 0, MAX_128_HASH_LEN);
    snprintf(buf, MAX_128_HASH_LEN, "%llu,%llu", Uint128Low64(*hash), Uint128High64(*hash));

    return String::New(buf);
}

uint64
to_uint64(const char* data, size_t len) {
    std::stringstream str;
    str.write(data, len);

    uint64 v;
    str >> v;
    return v;
}

void
to_uint128(uint128* v, const char* data, size_t len) {
    std::stringstream str;
    uint64 i;

    char* sep = strchr(data, (int)',');
    if(sep == NULL) {
        str.write(data, len);
        str >> i;
        v->first = i;
        v->second = 0;

    } else {
        int pos = (int)(sep - data);
        str.write(data, pos);
        str >> i;
        v->first = i;

        str.clear();
        str.write(sep+1, len - pos - 1);
        str >> i;
        v->second = i;
    }
}

Handle<Value>
node_CityHash64(const Arguments& args) {
    HandleScope scope;

    int args_len = args.Length();
    if(args_len == 0 || args_len > 3) {
        return ThrowException(String::New("Invalid arguments."));
    }

    String::Utf8Value data(args[0]->ToString());
    const char* str = *data;
    size_t len = data.length();

    uint64 hash;

    if(args_len == 1) {
        hash = CityHash64(str, len);
    } else if(args_len == 2) {
        String::AsciiValue seedString(args[1]->ToString());
        uint64 seed = to_uint64(*seedString, seedString.length());

        hash = CityHash64WithSeed(str, len, seed);
    } else if(args_len == 3) {
        String::AsciiValue seed0String(args[1]->ToString());
        String::AsciiValue seed1String(args[2]->ToString());
        uint64 seed0 = to_uint64(*seed0String, seed0String.length());
        uint64 seed1 = to_uint64(*seed1String, seed1String.length());

        hash = CityHash64WithSeeds(str, len, seed0, seed1);
    }

    return scope.Close(stringify_hash(hash));
}

Handle<Value>
node_CityHash128(const Arguments& args) {
    HandleScope scope;

    int args_len = args.Length();
    if(args_len == 0 || args_len > 2) {
        return ThrowException(String::New("Invalid arguments."));
    }

    String::Utf8Value data(args[0]->ToString());
    const char* str = *data;
    size_t len = data.length();

    uint128 hash;

    if(args.Length() == 2) {
        uint128 seed;
        String::AsciiValue seedString(args[1]->ToString());
        to_uint128(&seed, *seedString, seedString.length());

        hash = CityHash128WithSeed(str, len, seed);
    } else {
        hash = CityHash128(str, len);
    }

    return scope.Close(stringify_hash(&hash));
}

Handle<Value>
node_CityHashCrc128(const Arguments& args) {
    HandleScope scope;

    int args_len = args.Length();
    if(args_len == 0 || args_len > 2) {
        return ThrowException(String::New("Invalid arguments."));
    }

    String::Utf8Value data(args[0]->ToString());
    const char* str = *data;
    size_t len = data.length();

    uint128 hash;

    if(args.Length() == 2) {
        uint128 seed;
        String::AsciiValue seedString(args[1]->ToString());
        to_uint128(&seed, *seedString, seedString.length());

        hash = CityHashCrc128WithSeed(str, len, seed);
    } else {
        hash = CityHashCrc128(str, len);
    }

    return scope.Close(stringify_hash(&hash));
}

extern "C" void
init (Handle<Object> target) {
    HandleScope scope;
    target->Set(String::New("hash64"), FunctionTemplate::New(node_CityHash64)->GetFunction());
    target->Set(String::New("hash128"), FunctionTemplate::New(node_CityHash128)->GetFunction());
    target->Set(String::New("crc128"), FunctionTemplate::New(node_CityHashCrc128)->GetFunction());
}
