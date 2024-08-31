#pragma once
#include <iostream>

#define RLOG(exp) \
{ \
    std::cout << exp << std::endl; \
}

#define RERRLOG(exp) \
{ \
    std::cerr << exp << std::endl; \
}