namespace omvll {

using EncRoutineFn = void(char *Out, const char *In, unsigned long long Key, int Len);
EncRoutineFn *getEncodeRoutine(unsigned Idx);    
const char *getDecodeRoutine(unsigned Idx);
unsigned getNumEncodeDecodeRoutines();

}
    