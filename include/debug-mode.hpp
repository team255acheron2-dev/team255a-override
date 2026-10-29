#include "api.h" //Pros Everything (brain, others)
#include "EZ-Template/api.hpp" //EZ Template stuff
#include "menu.hpp"
#include <atomic>

//This file is meant to house the function/class definition for my debug mode menu
//I'll explain the concept in the cpp file
//This is seated above the api stuff and all that, but below the menu
//The menu will have a utility bar item to go to debug mode which will call the function in here
//Until the debug mode is exited and throws back to it

#pragma once
#ifndef DEBUG-MODE_HPP
#define DEBUG-MODE_HPP

inline std::atomic<bool> debugLock{false}; //The atomic to tell if the debug screen is open. Defaults to false (not open)

class debugMenuItem : public brainSpacing {
    public:
    debugMenuItem(std::string name) {
        this->name = name;
    }

    private:
    
};

struct debugMenu {
    int rows = 2;
    std::vector<debugMenuItem> saveItems;
};

extern void initDebug(debugMenu); //The function declaration for the initDebug function


#endif