/*                  Abstract Machine AMF+1

Abstract Machine AMF+1 is a 16bit computer architecture with 2**16=64K bytes
of memory (RAM). Each memory "word" is also 16 bits so there are 32K memory
addresses.  The memory is divided into the following segments:

Code Segment: Address 0-4095 (4K)
Stack Segment: Address 4096-12287  (8K)
Heap Segment: Address 65535 down to 12288 (20K)

CPU Architecture:

  The CPU of AMF+1 consists of an Arithmetic Logic Unit (ALU) capable of
  16-bit operations "add", "sub", "mult" and "div" as well as "mov" and "nop"

  The CPU contains 8 registers.  Six of these are accessible from software
  while two others are internal (software cannot directly read or change
  their values directly).  

    AX  : Accumulator register
    BX  : Alternate Accumulator register
    CX  : Counter register
    SP  : Stack pointer register
    BP  : Stack base register
    MA  : Memory address register

    PC  : program counter register  (internal)
    IR  : instruction register (internal)

Instruction Set Architecture:

ALU Instructions:

  opcode src dst   : operation on source operand src and destination dst.

  The opcode can be add, sub, mult or div.  The dst must name a (non-internal)
  register.  The src operand can be "register" or "immediate", meaning a
  constant.  For examples,

  sub ax bx  :  has effect bx = bx - ax;
  mult 3  cx :  has effect cx = cx * 3, 3 is an "immediate operand"

  ** the maximum value of an immediate operand is 255 (8 bits).  This is
  because the entire instruction must also fit into a 16-bit word.

  The div instruction calculates both the quotient and remainder, and
  always stores the remainder in register cx:

  div 2 ax :  cx=ax%2; ax = ax/2;

  If the dst operand is also cx, then the quotient is discarded.

  All alu operations work on 16-bit two's complement integers (int16_t),
  but immediate operands cannot be signed. 
    mov 0 ax
    sub 1 ax   
  has the effect of placing -1 in ax


Other CPU-Bound Instructions:

  mov src dst : "moves" source operand to destination.  The src can be
  immediate or register, the dst can only be a register (non-internal)

  nop:  does nothing at all, but it has an important role.


Memory Instructions:

  push src : pushes the (register or immediate source) onto the stack,
             incrementing the SP register.

  pop dst  : pops the contents of the top-of-stack into the dst register,
             decreasing the SP register.

  store src : stores the value of (register or immediate src) into the
             memory location indicated by the ma (memory address) register.
             The only way to address memory is to place the address into ma.

  load dst  : loads the value of memory at address held in the ma register 
             into the dst register.

  In the case of push and store, an immediate source operand is allowed to
  be 11 bits in size, with a value 0-2047 (2K).


Branch Instructions:

  jmp target : unconditional jump to instruction at memory location indicated
               by target.  By "jump" we mean setting the value of PC to the
               target.  Valid values of target are 1-4095 (within the
               code segment).  Usually, programs always start at memory 
               address 1.
  jnz target : jump to target instruction address if the value in the cx
              (counter) register is NOT ZERO.
  jz target :  jump to target instruction address if the value in the cx
              (counter) register IS ZERO.
  jn target :  jump to target instruction address if the value in the cx
              (counter) register IS NEGATIVE
  call target : pushes the current value of PC on the stack, then jump to
               to target instruction
  ret         : pops the value on top of the stack into PC and continue
               executing the instruction at address PC+1.

  
The PC register holds the address of the next instruction to execute.  After
the execution of each instruction, the PC is automatically incremented by one.

The instruction to execute is loaded into the IR (instruction register).


INSTRUCTION FORMAT:

  All instructions are stored in single 16-bit words (uint16_t).
  Since there are exactly 16 instructions, the first 4 bits (most significant
  bits) of the instruction stores the "opcode".  The opcode of each instruction
  is defined by their index in the following array:

  const string Instruction[16] = {
    "nop","add","sub","mult","div","push","pop","mov","load","store",
    "jmp","jnz","jz","jn","call","ret"};

  For example, the opcode for "mult" is 3.

  Depending on the instruction type, the other 12 bits of the instruction
  are interpreted as follows:

  The lowest (least significant) 3 bits identifies the destination register,
  for those instructions that require one.  Registers are identified by their
  indices in the following array:

  const string Register[6] = {"ax","bx","cx","sp","bp","ma"};

  So if the last 3 bits holds value 101 (5), it identies the ma register.

  IR bits usage for ALU instructions and the mov instruction:

     0123 4 56789ABC DEF
    ---------------------
    | op |i|     src|dst|  
    ---------------------

  op = opcode
  i = 1 if src is a register operand, i=0 if src is immediate.
  src : 8-bit immediate or register operand.  In case of a register src
        operand, only the lower 3 bits of these 8 bits (bits ABC) are used.
  dst : destination register.
  
  The pop and load instructions have the same format, except the src fields
  (bits 4-C) are not used.

  The push and store instructions have the same format, except that the
  lowest 3 bits (DEF) can also be used for an 11-bit immediate src operand.
  In case these instructions have a register src, then bits ABC will still
  represent the register.

  The branch instructions use all 12 bits after the 4 bit opcode (bits 4-F)
  to represent the target memory address.  12 bits can address 4K locations,
  which is exactly the size of the Code Segment of RAM.


Consult the sample programs for further illustration of how the instructions
work.  AMF+1 is a simple architecture but it has the essential flavors of 
a working computer and instruction set.

An "abstract machine" is only the specification of an architecture.
A "virtual machine" is the software implementation of an abstract machine.
*/

// VMF+1 implementation of AMF+1 (skeleton to be completed by students)
// compile with g++ vmfplusone_skeleton.cpp fponeasm.o

#include<iostream>
#include<string>
#include<cstdlib>
#include<cstring>
#include<cstdio>
#include<functional>
using namespace std;

static const string Instruction[16] = {
  "nop","add","sub","mult","div","push","pop","mov","load","store",
  "jmp","jnz","jz","jn","call","ret"};
static const string Register[6] = {"ax","bx","cx","sp","bp","ma"};

static const int MEMSIZE = 65536/2;    //2*32K = 64K = 2**16

const int16_t AX = 0;
const int16_t BX = 1;
const int16_t CX = 2;
const int16_t SP = 3;
const int16_t BP = 4;
const int16_t MAR = 5;

static bool TRACE = 1;

// to be imported from fponeasm.o :
char* cstr(string s);
uint16_t assemble(char input[]);

// to be implemented after main
void print_inst(uint16_t inst);
  

////// struct for virtual machine
struct vmfplusone
{
  int16_t RAM[MEMSIZE]; // 2*32*1024 = 64K RAM
  // uint8_t ROM[16*1024]
  uint16_t IR; // instruction register
  uint16_t PC; // program counter register
  bool IDLE;   // machine IDLE flag (1=true), not currently used
  int16_t REG[8]; // registers ax, bx, cx, sp, bp, mar

  static constexpr int CodeSegment = 0;     // 4K code segment
  static constexpr int StackSegment = 4096;
  static constexpr int StackLimit = 12*1024;  // 8K stack
  // Heap memory will grow downwards from max mem addr towards StackLimit
  static constexpr int HEAPBASE = 65535;

  vmfplusone() { // constructor
    IR = 0;
    PC = 1;
    IDLE = 1;
    memset(RAM,0,MEMSIZE); // zero memory contents
    memset(REG,0,16);       // zero register contents
    REG[SP] = StackSegment; // lowest possible value of sp register
  } // constructor

  // decode 8-bit source operand from IR register, you will have to
  // to write similar code to decode 11 and 12 bit operands.
  int16_t decode_src_8() {
    int16_t src;
      if (IR & 0x0800) { // register operand
        return REG[ (IR & 0x07f8) >> 3 ];
      }
      else {  // immediate operand for next 8 bits
        return (IR & 0x07f8) >> 3;
      }
  }//decode_src

  int16_t decode_src_11(){
    int16_t src;
      if (IR & 0x0800){
        return REG[(IR & 0x07f8) >> 3];
      }
      else {
        return (IR & 0x07ff);
      }
  }

  // decode destination register operand: value must index a register
  uint16_t decode_dst() { return IR % 8; }

  // write (outside struct) 16 functions, one for each instruction
  void nop();
  void add();
  void sub();
  void mult();
  void div();
  void push();
  void pop();
  void mov();
  void load();
  void store();
  void jmp();
  void jnz();
  void jz();
  void jn();
  void call();
  void ret();
  // see code samples after "main"


  // note: this function increments PC at the end.
  void execute_instruction()
  { 
    IR = RAM[CodeSegment+PC];   // load instruction register
    uint16_t opcode = IR >> 12; // decode opcode
    // dispatch vector of functions
    static function<void()> ops[] = {
      [&](){nop();},
      [&](){add();},
      [&](){sub();},
      [&](){mult();},
      [&](){div();},
      [&](){push();},
      [&](){pop();},
      [&](){mov();},
      [&](){load();},
      [&](){store();},
      [&](){jmp();},
      [&](){jnz();},
      [&](){jz();},
      [&](){jn();},      
      [&](){call();},
      [&](){ret();}
    };  
    ops[opcode]();   // dispatch and call function implementing opcode
    PC++;
  }// execute instruction


  void load_instruction(uint16_t inst) {
    RAM[PC++] = inst;
  }

  void status() // prints status
  {
    printf("ax=%d, bx=%d, cx=%d, sp=%d, bp=%d, ma=%d, pc=%d",REG[AX],REG[BX],REG[CX],(uint16_t)REG[SP],(uint16_t)REG[BP],(uint16_t)REG[MAR],(uint16_t)PC);
    if (REG[SP]>StackSegment) printf(", tos=%d",RAM[REG[SP]-1]);
    printf("\n");
  }//status
  
  void run(uint16_t start, uint16_t limit) { // run until max pc value = limit
    PC = start;
    while (PC<limit) {
      if (TRACE) {
        print_inst(RAM[PC]);  cout << ":\t";
      }
      execute_instruction();
      if (TRACE) status();
    }
  }//run with trace option
}; //vmfplusone struct


static const string err_message[] = {
  "Unspecified Error"
  "Stack Overflow",
  "Stack Underflow",
  "Stack Pointer Corruption",
  "Illegal Opcode",
  "Invalid Operand"
};

////////////main
int main(int argc, char* argv[])
{
  vmfplusone VM; // instance of virtual machine
  // load program into machine
  string line;  // input line
  uint16_t startpc = VM.PC;
  int linenum = 0;
  try {
   while (std::getline(std::cin,line)) {
    linenum++;
    if (line[0]=='.') break;
    if (line[0]=='\n' || line[0]=='\r' || line[0]=='#' || line.size()<3)
      {continue;}
    uint16_t inst = assemble(cstr(line));
    VM.load_instruction(inst);
   }//while getline
  } catch(int ec) {
    cout << err_message[ec] << ", line " << linenum << endl;
    linenum = -1; // signals error
  }//try-catch
  uint16_t endpc = VM.PC;
  TRACE = 1;
  try {
    if (endpc > startpc && linenum>=0) VM.run(startpc,endpc);
  } catch (int ec) { cout << err_message[ec] << " at PC=" << VM.PC << endl; }
  return 0;
}//main



///////////////////////////////////////////////////////////////////////////
//////////// complete the implementation of struct vmfplusone  ////////////

  // write a series of 16 functions, one for each instruction

  // Your functions must check for invalid conditions including:
  //   stack over/underflow. Valid REG[SP] range is 4096-12287
  //   invalid register index (valid indices are 0-5)
  //   invalid memory address in ma register (REG[MA]): this value
  //     cannot be negative: the valid range for REG[MA] is 0-32767
  //     Your program should also print a warning if ma is addressing
  //     the code segment (0-4095): cerr << "nasty hacker alert!"
  //   the ret instruction pops an address into PC that's not within the
  //     code segment (0-4095). This is a sure sign of a virus.
  // In case of violations, print an error message and throw an exception.

void vmfplusone::nop() { }

void vmfplusone::add() {    // must decode operands from IR
    int16_t src = decode_src_8();
    uint16_t dst = decode_dst();
    REG[dst] += src;
}//add

  // stub functions must be completed  
  void vmfplusone::sub() {   // must decode operands from IR
    int16_t src = decode_src_8();
    uint16_t dst = decode_dst();
    REG[dst] -= src;
  }
  void vmfplusone::mult() {   // must decode operands from IR
    int16_t src = decode_src_8();
    uint16_t dst = decode_dst();
    REG[dst] = REG[dst] * src;
  }
  void vmfplusone::div() {   // must decode operands from IR
    int16_t src = decode_src_8();
    uint16_t dst = decode_dst();
    int16_t temp = REG[dst];
    REG[dst] =  temp/ src; //src / temp;
    REG[2] = temp % src;
  }    

  void vmfplusone::push() {
    int16_t src = decode_src_11();
    RAM[REG[SP]++] = src;
  }//push

  void vmfplusone::pop() {
    uint16_t dst = decode_dst();
    if (REG[SP] > StackSegment){
      REG[SP]--;
      REG[dst] = RAM[REG[SP]];
    }
    else throw 2;
  }//pop

  void vmfplusone::mov() {
    uint16_t dst = decode_dst();
    uint16_t src = decode_src_8();
    REG[dst] = src;
  }

  void vmfplusone::load() { // load from [mar] to dst
    uint16_t dst = decode_dst();
    if (REG[MAR] < StackSegment)throw 2;  
    REG[dst] = RAM[REG[MAR]];
  }//load

  void vmfplusone::store() {  // store src operand to memory
    uint16_t src = decode_src_11();
    RAM[REG[MAR]] = src;
    }//store

  void vmfplusone::jmp() { // here operand is a 4K mem addr, 12 bits
    uint16_t src = decode_src_11();
    if (src > 0 || src < 4095){
      src--;
      PC = src;
    }
    else throw 5;
  }
  void vmfplusone::jnz() {  
    if (REG[CX] != 0) jmp();
  }  
  void vmfplusone::jz() {
    if (REG[CX] == 0) jmp();
  }  
  void vmfplusone::jn() {
    uint16_t src = decode_src_11();
    if (REG[CX] < 0) jmp();
  }
  void vmfplusone::call() {
    if (REG[SP]-1 >= StackSegment){
      RAM[REG[SP]++] = PC;
      jmp();
    }
    else
      throw 2;
  }
  void vmfplusone::ret() { // watch out for hackers!
    if (REG[SP]-1 >= StackSegment){
      PC = RAM[--REG[SP]];
    }
    else
      throw 2;
  }

//// complete the following procedure to produce the expected output:
//// print the instruction in readable form
void print_inst(uint16_t inst)
{
}//decode and print instruction such as "add ax bx" (no endl please)


/*
Compile and run program as follows:

g++ -c -O3 fponeasm.cpp   (this produces fponeasm.o)
g++ -O3 vmfplusone_skeleton.cpp fponeasm.o -o vm16  (produces vm16 or vm16.exe)

Run vm16 or vm16.exe either by entering instructions on the command line, 
terminating input with a .  or by feeding it a file:

./vm16 < arith.am16
*/
