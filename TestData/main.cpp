/*
* test 
* block
* comment
*/
#include <iostream>

// single line comment
int main()
{
	// comment only
	std::cout << "Hello World!"; // code and comment line

	// this is really an int, but it shouldn't count as a block comment
	std::cout << '/*';

	// this shouldn't count as a block comment either
	std::cout << "/*";
	std::cout << "*/"; /*
	
		block comment

	*/
}

/*
Solution TestData
Total Projects: 1
Total Files:    1
Total Lines:    33
Blank Lines:    4
Comment Lines:  23
*/