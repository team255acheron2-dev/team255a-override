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

    bool isToggled() const {
        return toggled;
    }

    void setToggle(bool newState) {
        toggled = newState;
    }

    std::string nameIs() const {
        return name;
    }

    void setBoxColor(int newColor) {
        color = newColor;
    }

    void setTextColor(int newColor) {
        textColor = newColor;
    }

    void drawBoxUpdated() const { //Call when drawing box after making background
        pros::screen::set_pen(color); //Box color
        pros::screen::fill_rect(pos.left, pos.top, pos.right, pos.bottom); //Draws rectangle
        pros::screen::set_pen(textColor); //Text color (dependent on current screen)

        pros::text_format_e_t size = pros::E_TEXT_LARGE; //Large by default

        u_int16_t xSize = pos.right - pos.left; //Non-negative number 16 bits big
        u_int8_t ySize = pos.bottom - pos.top; //Non-negative number 8 bits big (0-255)

        if (ySize < 100||xSize < 200) size = pros::E_TEXT_MEDIUM; //Narrows down to medium, and if too small
        if (ySize < 50||xSize < 100) size = pros::E_TEXT_SMALL; //Narrow down to small size

        pros::screen::print(size, (pos.left + 4), ((pos.top + pos.bottom)/2), "%s", name); //Print in the specified size.
    }

    private:
    std::vector<std::string> printData; //Saves the data to print into a vector
    bool toggled = false;
    int color = 0x000000;
    int textColor = 0xFFFFFF;
};

struct debugMenu {
    int itemRows = 3;
};

extern void initDebug(debugMenu); //The function declaration for the initDebug function
inline std::vector<debugMenuItem> debugOptions;

extern void updateDebugItems();

#endif