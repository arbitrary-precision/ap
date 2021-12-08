# Arbitrary Precision (AP)
Cross-platform and cross-standard header-only arbitrary precision arithmetic library.
Currently it offers integer types that behave as if they are built-in int, but able to store arbitrarily huge values. 

## Usage
1. Clone the repo.  
2. include ap.hpp header.  
3. Use either ap_int&lt;BITSIZE&gt; or ap_uint&lt;BITSIZE&gt; template type, parametrized with size in bits.

## Interface and features
Two types are available out of the box: </br>
ap_int&lt;BITSIZE&gt; - signed integer of size BITSIZE with two's complement representation.</br>
ap_uint&lt;BITSIZE&gt; - unsigned integer of size BITSIZE.</br>
Both behave and offer interface of the basic integer type (without advanced algorithms).</br>
* AP integer can be initialized from any built-in integer type or any other AP integer type.</br>
* AP integer can be initialized from binary, octal, decimal and uppercase hexadecimal strings. Base determined automatically, if it is set to 0.</br>
* Custom digits may be used to represent integers, see parameters set() and str() function.</br>

### Operators
All arithmetic, comparison and bitwise operators are available (as for int).</br>
The behavior is as follows:
* BiTSIZE aligned to size of AP_WORD.
* comparison, increment, decrement, sign change, addition, subtraction, multiplication behave as expected and return the result of the corresponding operation.
* bitwise operations mock two's complement behavior.
* Division by zero triggers native system behavior.
* Right shift by count of bytes greater than BITSIZE yields 0.
* Left shift by count of bytes greater than BITSIZE yields -1 for negative values, 0 in the other case.
* Output to ostream is base-aware, so it checks for iso_base hex, dec, oct flags.
* Hexadecimal strings are always uppercase, be it initialization or output. You can specify string "0123456789abcdef" if you want lowercase.


Note: For those who are concerned, it is possible to switch compilation approach and compile .cpp files separately.</br>
Simply define AP_USE_SOURCES before including ap.hpp, or via compiler options. However, you will have to add .cpp files to your project by yourself.</br>
Another option is to use different types for computation and storage of long integer, define AP_WORD and AP_DWORD with unsigned types of your choice, e.g.:</br>
`-DAP_WORD=unsigned\ char`</br>
`-DAP_DWORD=unsigned\ short`</br>
Note that AP_DWORD must be exactly twice as big as AP_WORD, both must be unsigned.</br>
