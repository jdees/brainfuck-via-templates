/// Author: Jonathan Dees
/// All rights reserved.

// Brainfuck language specification
//
// > => ++ptr;
// < => --ptr;
// + => ++*ptr;
// - => --*ptr;
// . => putchar(*ptr);
// , => *ptr = getchar();
// [ => while (*ptr) {
// ] => }


#include <cinttypes>
#include <cstdio>

using bf_data_t = char;
const bf_data_t data_zero = 0;
using bf_command_t = char;
const bf_command_t command_terminal = '\0';
const bf_command_t command_next = 127;


template <bf_command_t... Command> struct Code {};
template <bf_data_t... LData> struct TapeLeft {};
template <bf_data_t... RData> struct TapeRight {}; //< first character is the current pointed to value
template <bf_data_t... IData> struct Input {};
template <bf_data_t... OData> struct Output {};

template <bool DoFail> struct FailInput { static_assert(!DoFail, "missing input when executing ,"); };
template <bool DoFail> struct FailInternalError { static_assert(!DoFail, "internal erorr on execution"); };
template <bool DoFail> struct FailMissingEnclosingBracket { static_assert(!DoFail, "missing enclosing bracket"); };
template <bool DoFail> struct FailMissingOpeningBracket { static_assert(!DoFail, "missing opening bracket"); };

template < typename CodeStack, bf_command_t C, typename TheTapeLeft, typename TheTapeRight, typename TheInput, typename TheOutput > struct Program
{
  using run = FailInternalError<true>();
};

// finished executing
template < typename TheCodeStack, typename TheTapeLeft, typename TheTapeRight, typename TheInput, typename TheOutput >
struct Program< TheCodeStack, command_terminal, TheTapeLeft, TheTapeRight, TheInput, TheOutput >
{
  using run = TheOutput;
};


template < typename TheCodeLeft, typename TheCodeRight > struct CodeStack;

template < typename TheCodeStack, bool TheIsZero > struct NextCodeStack;

template < typename TheCodeLeft, bool TheIsZero > struct NextCodeStack< CodeStack< TheCodeLeft, Code<> >, TheIsZero >
{
  static const bf_command_t command = command_terminal;
  using stack = CodeStack< TheCodeLeft, Code<> >;
};

template < bf_command_t... CL, bf_command_t CRHead, bf_command_t... CRTail, bool TheIsZero > struct NextCodeStack< CodeStack< Code< CL...>, Code< CRHead, CRTail... > >, TheIsZero  >
{
  static const bf_command_t command = CRHead;
  static_assert(command != '[', "internal compile error"); //< compiler should prefer specialization
  static_assert(command != ']', "internal compile error"); //< compiler should prefer specialization
  using stack = CodeStack< Code< CRHead, CL... >, Code< CRTail... > >;
};

template < bf_command_t... CL, bf_command_t... CRTail > struct NextCodeStack< CodeStack< Code< CL... >, Code< '[', CRTail... > >, true  >
{
  using next_code_stack = NextCodeStack< CodeStack< Code< '[', CL... >, Code< CRTail... > >, true >;
  static const bf_command_t command = next_code_stack::command;
  using stack = typename next_code_stack::stack;
};

template < class CodeStack, int NumBrackets > struct ScanForward;
template < class CodeLeft, int TheNumBrackets > struct ScanForward< CodeStack< CodeLeft, Code<> >, TheNumBrackets >
{
  using result = FailMissingEnclosingBracket<true>;
};

template < bf_command_t... CL, bf_command_t CRHead, bf_command_t... CRTail, int TheNumBrackets > struct ScanForward < CodeStack< Code<CL...>, Code<CRHead, CRTail...> >, TheNumBrackets >
{
  using result = typename ScanForward< CodeStack< Code< CRHead, CL... >, Code< CRTail... > >, TheNumBrackets >::result;
};
template < bf_command_t... CL, bf_command_t... CRTail, int TheNumBrackets > struct ScanForward < CodeStack< Code<CL...>, Code<'[', CRTail...> >, TheNumBrackets >
{
  using result = typename ScanForward< CodeStack< Code< '[', CL... >, Code< CRTail... > >, TheNumBrackets + 1 >::result;
};
template < bf_command_t... CL, bf_command_t... CRTail, int TheNumBrackets > struct ScanForward < CodeStack< Code<CL...>, Code<']', CRTail...> >, TheNumBrackets >
{
  using result = typename ScanForward< CodeStack< Code< ']', CL... >, Code< CRTail... > >, TheNumBrackets - 1 >::result;
};
template < bf_command_t... CL, bf_command_t... CRTail > struct ScanForward < CodeStack< Code<CL...>, Code<']', CRTail...> >, 1 >
{
  using result = CodeStack< Code< ']', CL... >, Code< CRTail... > >;
};

template < bf_command_t... CL, bf_command_t... CRTail > struct NextCodeStack< CodeStack< Code< CL... >, Code< '[', CRTail... > >, false  >
{
  using code_stack_scan = typename ScanForward < CodeStack< Code< '[', CL... >, Code< CRTail... > >, 1 >::result;
  using next_code_stack = NextCodeStack< code_stack_scan, false >;
  static const bf_command_t command = next_code_stack::command;
  using stack = typename next_code_stack::stack;
};


template < bf_command_t... CL, bf_command_t... CRTail > struct NextCodeStack< CodeStack< Code< CL... >, Code< ']', CRTail... > >, false  >
{
  using next_code_stack = NextCodeStack< CodeStack< Code< ']', CL... >, Code< CRTail... > >, false >;
  static const bf_command_t command = next_code_stack::command;
  using stack = typename next_code_stack::stack;
};

template < class CodeStack, int NumBrackets > struct ScanBackward;
template < class CodeRight, int TheNumBrackets > struct ScanBackward< CodeStack< Code<>, CodeRight >, TheNumBrackets >
{
  using result = FailMissingOpeningBracket<true>;
};
template < bf_command_t CLHead, bf_command_t... CLTail, bf_command_t... CR, int TheNumBrackets > struct ScanBackward < CodeStack< Code<CLHead, CLTail...>, Code<CR...> >, TheNumBrackets >
{
  using result = typename ScanBackward< CodeStack< Code< CLTail... >, Code< CLHead, CR... > >, TheNumBrackets >::result;
};
template < bf_command_t... CLTail, bf_command_t... CR, int TheNumBrackets > struct ScanBackward < CodeStack< Code<']', CLTail...>, Code<CR...> >, TheNumBrackets >
{
  using result = typename ScanBackward< CodeStack< Code< CLTail... >, Code< ']', CR... > >, TheNumBrackets + 1 >::result;
};
template < bf_command_t... CLTail, bf_command_t... CR, int TheNumBrackets > struct ScanBackward < CodeStack< Code<'[', CLTail...>, Code< CR...> >, TheNumBrackets >
{
  using result = typename ScanBackward< CodeStack< Code< CLTail... >, Code< '[', CR... > >, TheNumBrackets - 1 >::result;
};
template < bf_command_t... CLTail, bf_command_t... CR > struct ScanBackward < CodeStack< Code<'[', CLTail...>, Code< CR...> >, 1 >
{
  using result = CodeStack< Code< CLTail... >, Code< '[', CR... > >;
};


template < bf_command_t... CL, bf_command_t... CRTail > struct NextCodeStack< CodeStack< Code< CL... >, Code< ']', CRTail... > >, true  >
{
  using fail = FailInternalError<true>;
  using code_stack_scan_back = typename ScanBackward < CodeStack< Code<  CL... >, Code< ']', CRTail... > >, 1 >::result;
  using next_code_stack = NextCodeStack< code_stack_scan_back, true >;
  static const bf_command_t command = next_code_stack::command;
  using stack = typename next_code_stack::stack;
};


// pop next command to execute
template < typename TheCodeStack, typename TheTapeLeft, bf_data_t RDataHead, bf_data_t... RDataTail, typename TheInput, typename TheOutput >
struct Program< TheCodeStack, command_next, TheTapeLeft, TapeRight<RDataHead, RDataTail...>, TheInput, TheOutput >
{
  using next_code_stack = NextCodeStack< TheCodeStack, RDataHead != data_zero >;
  using run = typename Program< typename next_code_stack::stack, next_code_stack::command, TheTapeLeft, TapeRight<RDataHead, RDataTail...>, TheInput, TheOutput >::run;
};

template < typename TheCodeStack, typename TheTapeLeft, typename TheInput, typename TheOutput >
struct Program< TheCodeStack, command_next, TheTapeLeft, TapeRight<>, TheInput, TheOutput >
{
  using next_code_stack = NextCodeStack< TheCodeStack, data_zero != data_zero >;
  using run = typename Program< typename next_code_stack::stack, next_code_stack::command, TheTapeLeft, TapeRight<data_zero>, TheInput, TheOutput >::run;
};



template < typename TheCode, bf_command_t TheC, typename TheTapeLeft, typename TheInput, typename TheOutput >
struct Program< TheCode, TheC, TheTapeLeft, TapeRight<>, TheInput, TheOutput >
{
  using run = Program<TheCode,TheC,TheTapeLeft,TapeRight< data_zero >, TheInput, TheOutput >;
};

// command '.'
template < typename TheCode, typename TheTapeLeft, bf_data_t RDataHead, bf_data_t... RDataTail, typename TheInput, bf_data_t... OData >
struct Program< TheCode, '.', TheTapeLeft, TapeRight<RDataHead, RDataTail...>, TheInput, Output< OData... > >
{ using run = typename Program< TheCode, command_next, TheTapeLeft, TapeRight<RDataHead,RDataTail...>, TheInput, Output< OData..., RDataHead> >::run; };

// command ','
template < typename TheCode, typename TheTapeLeft, bf_data_t RDataHead, bf_data_t... RDataTail, bf_data_t IDataHead, bf_data_t... IDataTail, typename TheOutput >
struct Program< TheCode, ',', TheTapeLeft, TapeRight<RDataHead, RDataTail...>, Input<IDataHead,IDataTail...>, TheOutput >
{ using run = typename Program< TheCode, command_next, TheTapeLeft, TapeRight<IDataHead,RDataTail...>, Input<IDataTail...> , TheOutput >::run; };

// Report failure: missing Input
template < typename TheCode, typename TheTapeLeft, typename TheTapeRight, typename TheOutput >
struct Program< TheCode, ',', TheTapeLeft, TheTapeRight, Input<>, TheOutput >
{
  using run = FailInput<true>;
};


// command '+'
template < typename TheCode, typename TheTapeLeft, bf_data_t RDataHead, bf_data_t... RDataTail, typename TheInput, typename TheOutput >
struct Program< TheCode, '+', TheTapeLeft, TapeRight<RDataHead, RDataTail...>, TheInput, TheOutput >
{
  using run = typename Program< TheCode, command_next, TheTapeLeft, TapeRight< RDataHead + 1, RDataTail... >, TheInput, TheOutput >::run;
};

// command '-'
template < typename TheCode, typename TheTapeLeft, bf_data_t RDataHead, bf_data_t... RDataTail, typename TheInput, typename TheOutput >
struct Program< TheCode, '-', TheTapeLeft, TapeRight<RDataHead, RDataTail...>, TheInput, TheOutput >
{
  using run = typename Program< TheCode, command_next, TheTapeLeft, TapeRight< RDataHead - 1, RDataTail... >, TheInput, TheOutput >::run;
};

// command '>'
template < typename TheCode, bf_data_t... LData, bf_data_t RDataHead, bf_data_t... RDataTail, typename TheInput, typename TheOutput >
struct Program< TheCode, '>', TapeLeft<LData...>, TapeRight<RDataHead, RDataTail...>, TheInput, TheOutput >
{
  using run = typename Program< TheCode, command_next, TapeLeft<RDataHead, LData...>, TapeRight< RDataTail... >, TheInput, TheOutput >::run;
};

// command '<'
template < typename TheCode, bf_data_t LDataHead, bf_data_t... LDataTail, bf_data_t... RData, typename TheInput, typename TheOutput >
struct Program< TheCode, '<', TapeLeft<LDataHead, LDataTail...>, TapeRight<RData...>, TheInput, TheOutput >
{
  using run = typename Program< TheCode, command_next, TapeLeft< LDataTail...>, TapeRight< LDataHead, RData... >, TheInput, TheOutput >::run;
};

template < typename TheCode, bf_data_t... RData, typename TheInput, typename TheOutput >
struct Program< TheCode, '<', TapeLeft<>, TapeRight<RData...>, TheInput, TheOutput >
{
  using run = typename Program< TheCode, command_next, TapeLeft<>, TapeRight< data_zero, RData... >, TheInput, TheOutput >::run;
};


// printing helper
template <class T> struct PrintOutput;
template < bf_data_t first, bf_data_t... OData > struct PrintOutput< Output< first, OData... > > {
  void operator()() { putchar(first); PrintOutput< Output< OData... > >()(); }
};
template <> struct PrintOutput< Output<> > { void operator()() { putchar('\n'); } };
template < typename Output > void print_output(Output o=Output())
{
  PrintOutput< Output >()();
}



int main()
{
  {
    using the_code = Code<'<', '>', ',', '+', '.', '+', '+', '-', '-', '-', '.', '<', '.'>;
    using the_input = Input<'X'>;
    using the_tape_left = TapeLeft<'K'>;
    using the_tape_right = TapeRight<'1', '2'>;
    using the_output = Output<'H', 'E', 'L', 'L', 'O'>;
    print_output(Program< CodeStack< Code<>, the_code >, command_next, the_tape_left, the_tape_right, the_input, the_output >::run());
  }

  {
    using the_code = Code<'+', '+', '[', '-', '.', ']', '+', '+', '-', '.', '+', '+', '.'>;
    using the_input = Input<'X'>;
    using the_tape_left = TapeLeft<>;
    using the_tape_right = TapeRight<>;
    using the_output = Output<>;
    PrintOutput< Program< CodeStack< Code<>, the_code >, command_next, the_tape_left, the_tape_right, the_input, the_output >::run >()();
  }
  return 0;
}
