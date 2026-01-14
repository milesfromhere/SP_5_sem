#pragma once
#define OS12HANDEL void*

namespace OS12
{
    OS12HANDEL Init(); // Инициализация COM и создание объекта

    namespace Adder
    {
        double Add(OS12HANDEL h, double x, double y); 
        double Sub(OS12HANDEL h, double x, double y); 
    }

    namespace Multiplier
    {
        double Mul(OS12HANDEL h, double x, double y); 
        double Div(OS12HANDEL h, double x, double y); 
    }

    void Dispose(OS12HANDEL h); // Освобождение ресурсов
}
