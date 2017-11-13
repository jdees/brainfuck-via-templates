/// Author: Jonathan Dees
/// All rights resevered.

// Branfuck language specification
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
using bf_command_t = char;
const bf_data_t data_zero = 0;

struct Splitter {};

template <bf_command_t... Command> struct Code {};
template <bf_data_t... LData> struct TapeLeft {};
template <bf_data_t... RData> struct TapeRight {}; //< first character is the current pointed to value
template <bf_data_t... IData> struct Input {};
template <bf_data_t... OData> struct Output {};

template <bool DoFail> struct FailInput { static_assert(!DoFail, "missing input when executing ,"); };
template <bool DoFail> struct FailInternalError { static_assert(!DoFail, "internal erorr on execution"); };


template <  typename TheCode, typename TheTapeLeft, typename TheTapeRight, typename TheInput, typename TheOutput > struct Program
{
	using run = FailInternalError<true>();
};


template < typename TheTapeLeft, typename TheTapeRight, typename TheInput, typename TheOutput >
struct Program< Code<>, TheTapeLeft, TheTapeRight, TheInput, TheOutput >
{
	using run = TheOutput;
};

template < typename TheCode, typename TheTapeLeft, typename TheInput, typename TheOutput >
struct Program< TheCode, TheTapeLeft, TapeRight<>, TheInput, TheOutput >
{
	using run = typename Program<TheCode,TheTapeLeft,TapeRight< data_zero >, TheInput, TheOutput >;
};

// command '.'
template < bf_command_t... Command, typename TheTapeLeft, bf_data_t RDataHead, bf_data_t... RDataTail, typename TheInput, bf_data_t... OData >
struct Program< Code<'.', Command...>, TheTapeLeft, TapeRight<RDataHead, RDataTail...>, TheInput, Output< OData... > >
{ using run = typename Program< Code<Command...>, TheTapeLeft, TapeRight<RDataHead,RDataTail...>, TheInput, Output< OData..., RDataHead> >::run; };

// command ','
template < bf_command_t... Command, typename TheTapeLeft, bf_data_t RDataHead, bf_data_t... RDataTail, bf_data_t IDataHead, bf_data_t... IDataTail, typename TheOutput >
struct Program< Code<',', Command...>, TheTapeLeft, TapeRight<RDataHead, RDataTail...>, Input<IDataHead,IDataTail...>, TheOutput >
{ using run = typename Program< Code<Command...>, TheTapeLeft, TapeRight<IDataHead,RDataTail...>, Input<IDataTail...> , TheOutput >::run; };

// Report failure: missing Input
template < bf_command_t... Command, typename TheTapeLeft, typename TheTapeRight, typename TheOutput >
struct Program< Code<',', Command...>, TheTapeLeft, TheTapeRight, Input<>, TheOutput >
{ using run = FailInput<true>; };


// command '+'
template < bf_command_t... Command, typename TheTapeLeft, bf_data_t RDataHead, bf_data_t... RDataTail, typename TheInput, typename TheOutput >
struct Program< Code<'+', Command...>, TheTapeLeft, TapeRight<RDataHead, RDataTail...>, TheInput, TheOutput >
{ using run = typename Program< Code<Command...>, TheTapeLeft, TapeRight< RDataHead+1, RDataTail... > , TheInput, TheOutput >::run; };

// command '-'
template < bf_command_t... Command, typename TheTapeLeft, bf_data_t RDataHead, bf_data_t... RDataTail, typename TheInput, typename TheOutput >
struct Program< Code<'-', Command...>, TheTapeLeft, TapeRight<RDataHead, RDataTail...>, TheInput, TheOutput >
{ using run = typename Program< Code<Command...>, TheTapeLeft, TapeRight< RDataHead - 1, RDataTail... >, TheInput, TheOutput >::run; };


// printing helper
template <class T> struct PrintOutput;
template < bf_data_t first, bf_data_t... OData > struct PrintOutput< Output< first, OData... > > {
	void operator()() { putchar(first); PrintOutput< Output< OData... > >()(); }
};
template <> struct PrintOutput< Output<> > { void operator()() { putchar('\n'); } };
template < typename Output > void print_output(Output o=Output())
{
	PrintOutput< Output >()();
};

int main()
{
	using the_code = Code<',','+','.','+','+','-','-','-','.'>;
	using the_input = Input<'X'>;
	using the_tape_left = TapeLeft<>;
	using the_tape_right = TapeRight<'1','2'>;
	using the_output = Output<'H','E','L','L','O'>;

	print_output( Program< the_code, the_tape_left, the_tape_right, the_input, the_output >::run() );
    return 0;
}

