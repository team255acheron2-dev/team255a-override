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

    using type = std::string; //Abbreviates std::string to the name type
    template <typename... type> //Uses that type to create a fold of that type (fold expressions allow for dynamic parameters)
    debugMenuItem(std::string title, type... saveDataValues) { //Takes the name (will print first), and all string save data
        name = title; //Saves the name of the data internally

        auto storeData = [this](std::string currentData) { //Lambda expression to 'unpackage' the fold expression
            printData.push_back(currentData); //Saves each string to the vector of the save data
        };

        (storeData(saveDataValues), ...); //Calls the lambda to unfold the fold expression with each term
    }

    std::string getOutputData() {
        std::string finalString = "\n\n" + name;

        for (int i = 0; i < printData.size(); i++) {
            finalString += "\n" + printData[i];
        }

        return finalString;
    }

    bool isToggled() {
        return toggled;
    }

    void setToggle(bool newState) {
        toggled = newState;
    }

    std::string nameIs() {
        return name;
    }

    private:
    std::vector<std::string> printData; //Saves the data to print into a vector
    bool toggled = false;
};

struct debugMenu {
    int rows = 2;
    std::vector<debugMenuItem> saveItems;
};

extern void initDebug(debugMenu); //The function declaration for the initDebug function


#endif