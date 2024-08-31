#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "App/App.h"

int main() 
{
    Ruya::App app;

    try
    {
        app.Run();
    }catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}