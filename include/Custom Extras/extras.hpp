#include "iostream"
#include <cstdlib>

#pragma once
#ifndef extras_HPP
#define extras_HPP

namespace custom { //Has every custom thing I've made

    namespace num { //Namespace for number-related stuff
        //This is simple code that looks complicated because of how simple it is
        //In essence it is just the following

        template <typename convOutput, typename convInput> //A variable type I make that uses typename, and two types
        //Typename is a fill in a bland for a type. This can be replaced with int, float, double, char, etc.
        //That's what makes templates useful, I can use this for any and all number conversion

        inline convOutput convert(convInput input) { //Creates the function. It will output a conversion of a new input
            return static_cast<convOutput>(input); //Sets it to the inputted type
        } //End of function. Inline is saying it is const and static, and will never change across files
        //To call it, I need to just say wantedOutputType var (inputType can be int, float, etc)
        //Then later I do var = convNum<Output type>(input);
        //Overall, I and the reader need to remember, that name<output>(input)
        //The computer will handle the rest.
    }

    //I can add more custom stuff here
    //In namespaces if I want.
}

#endif