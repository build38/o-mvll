#include "omvll/passes/string-encoding/Routines.h"

namespace {

const char* DecodeRoutines[] = {
  R"delim(
    void decode(char *out, char *in, unsigned long long key, int size) {
      unsigned char *raw_key = (unsigned char*)(&key);
      for (int i = 0; i < size; ++i) {
        out[i] = in[i] ^ raw_key[i % sizeof(key)];
      }
    }
  )delim",
  R"delim(
    void decode(char *out, char *in, unsigned long long key, int size) {
      unsigned char *raw_key = (unsigned char*)(&key);
      for (int i = 0; i < size; ++i) {
        out[i] = in[i] ^ raw_key[i % sizeof(key)] ^ i;
      }
    }
  )delim",
};

void encode1(char *out, const char *in, unsigned long long key, int size) {
  unsigned char *raw_key = (unsigned char*)(&key);
  for (int i = 0; i < size; ++i) {
    out[i] = in[i] ^ raw_key[i % sizeof(key)];
  }
}

void encode2(char *out, const char *in, unsigned long long key, int size) {
  unsigned char *raw_key = (unsigned char*)(&key);
  for (int i = 0; i < size; ++i) {
    out[i] = in[i] ^ raw_key[i % sizeof(key)] ^ i;
  }
}

omvll::EncRoutineFn *EncodeRoutines[] = {
  &encode1,
  &encode2,
};

}

namespace omvll {

unsigned getNumEncodeDecodeRoutines() {
  return 2;
}

EncRoutineFn *getEncodeRoutine(unsigned Idx) {
  return EncodeRoutines[Idx];
}

const char *getDecodeRoutine(unsigned Idx) {
  return DecodeRoutines[Idx];
}

}
