#include <iostream>
#include "../SP12_LIB/SP12_LIB.h"

int main()
{

	try
	{
		// Инициализация двух независимых COM-объектов
		OS12HANDEL h1 = OS12::Init();
		OS12HANDEL h2 = OS12::Init();

		std::cout << "SP12::Adder::Add(h1, 2, 3) = " << OS12::Adder::Add(h1, 2, 3) << "\n";
		std::cout << "OS12::Adder::Add(h2, 2, 3) = " << OS12::Adder::Add(h2, 2, 3) << "\n";

		std::cout << "OS12::Adder::Sub(h1, 2, 3) = " << OS12::Adder::Sub(h1, 2, 3) << "\n";
		std::cout << "OS12::Adder::Sub(h2, 2, 3) = " << OS12::Adder::Sub(h2, 2, 3) << "\n";

		std::cout << "OS12::Multiplier::Mul(h1, 2, 3) = " << OS12::Multiplier::Mul(h1, 2, 3) << "\n";
		std::cout << "OS12::Multiplier::Mul(h2, 2, 3) = " << OS12::Multiplier::Mul(h2, 2, 3) << "\n";

		std::cout << "OS12::Multiplier::Div(h1, 2, 3) = " << OS12::Multiplier::Div(h1, 2, 3) << "\n";
		std::cout << "OS12::Multiplier::Div(h2, 2, 3) = " << OS12::Multiplier::Div(h2, 2, 3) << "\n";

		OS12::Dispose(h1);
		OS12::Dispose(h2);

	}
	catch (int e) { std::cout << "OS12: error = " << e << "\n"; }

}


