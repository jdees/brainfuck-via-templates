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


template <  typename TheCode, typename TheTapeLeft, typename TheTapeRight, typename TheInput, typename TheOutput > struct Program
{
	using run = Output< 'F', 'A', 'I', 'L' >;
};

namespace detail
{

template< typename TheTapeRight, typename TheOutput > struct OpOut;
template< bf_data_t... OData > struct OpOut< TapeRight<>, Output< OData... > > { using result = Output< OData..., data_zero >; };
template< bf_data_t RDataHead, bf_data_t... RDataTail, bf_data_t... OData > struct OpOut< TapeRight< RDataHead, RDataTail... >, Output< OData... > > { using result = Output< OData..., RDataHead >; };

template< typename TheTapeRight > struct OpPlus;
template< bf_data_t ODataHead, bf_data_t... ODataTail > struct OpPlus< TapeRight< ODataHead, ODataTail... > > { using result = TapeRight< ODataHead + 1, ODataTail... >; };
template< > struct OpPlus< TapeRight<> > { using result = TapeRight< data_zero + 1 >; };

} //< ns detail


template < typename TheTapeLeft, typename TheTapeRight, typename TheInput, typename TheOutput >
struct Program< Code<>, TheTapeLeft, TheTapeRight, TheInput, TheOutput >
{
	using run = TheOutput;
};

template < bf_command_t... Command, typename TheTapeLeft, typename TheTapeRight, typename TheInput, typename TheOutput >
struct Program< Code<'.', Command...>, TheTapeLeft, TheTapeRight, TheInput, TheOutput >
{
	using run = typename Program< Code<Command...>, TheTapeLeft, TheTapeRight, TheInput, typename detail::OpOut<TheTapeRight,TheOutput>::result >::run;
};

template < bf_command_t... Command, typename TheTapeLeft, typename TheTapeRight, typename TheInput, typename TheOutput >
struct Program< Code<'+', Command...>, TheTapeLeft, TheTapeRight, TheInput, TheOutput >
{
	using run = typename Program< Code<Command...>, TheTapeLeft, typename detail::OpPlus< TheTapeRight >::result , TheInput, TheOutput >::run;
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
};

int main()
{
	using the_code = Code<'+','.','+','+','.'>;
	using the_input = Input<'X'>;
	using the_tape_left = TapeLeft<>;
	using the_tape_right = TapeRight<'1','2'>;
	using the_output = Output<'H','E','L','L','O'>;

	print_output( Program< the_code, the_tape_left, the_tape_right, the_input, the_output >::run() );
    return 0;
}

