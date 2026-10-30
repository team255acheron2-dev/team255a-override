#include "api.h" //Pros Everything (brain, others)
#include "EZ-Template/api.hpp" //EZ Template stuff
#include "autons.hpp" //Autos
#include "liblvgl/lvgl.h" //Photos and colors
#include <vector> //Vectors
#include "Custom Extras/extras.hpp" //My conversions
#include <fstream> //File management
#include <atomic> //For the atomic later
#include "iostream" //I/O stream


//Goal of this major revision:
//Redo the menu with 3 sections (red, blue, utility)
//Plus calculations for autons, so all I have to do
//Is create the autons class with a string and two callbacks

//Apparently this idea of an auton selector can also be done using
//pros::lcd. But I don't know how that works right now. I'll look into it later.
//This file has been tested and is shown to work. If I ever switch to Lemlib I'm fried converting this.

#pragma once //Header guard
#ifndef MENU_HPP //Header check
#define MENU_HPP //The actual definition

//I make them static because I don't want to pollute the global workspace outside of this file (limits them to this file)
//I make them constant because they won't change, and they are integers
static const int BLACK = 0x000000; //Abbreviates black to its hexadecimal number
static const int BLUE = 0x0000FF; //Abbreviates blue to its hexadecimal number
static const int RED = 0xFF0000; //Abbreviates red to its hexadecimal number
static const int YELLOW = 0xFFFF00; //Abbreviates yellow to its hexadecimal number
static const int WHITE = 0xFFFFFF; //Abbreviates white to its hexadecimal number
static const int TRANSPARENT = 0xFFFFFF00; //The transparent color (extra 00 at start to set a to 0)
//This makes it easier to read later

extern std::atomic<bool> debugLock;

using namespace std; //std namespace is used

extern void drawImage(); //The drawImage function

extern int color; //variable to use as color
extern int textColor; //Variable for text color

class brainSpacing { //Simple class with the name and pos variables, as well as point checking and drawing functions
    public: //Section that gets put into public for the children

    bool containsPoint(int x, int y) const { //Run when brain clicked
        //Checks x point, checks y point. X for l & r, Y for b & t
        return ((x >= pos.left&&x <= pos.right)&&
                (y <= pos.bottom&&y >= pos.top)); //Returns if all are true
    }

    void drawBox() const { //Call when drawing box after making background
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

    void setPosition(int left, int right, int top, int bottom) { //Sets the member's position to the inputted numberds
        pos.left = left; //Sets left to left
        pos.right = right; //Sets right to right
        pos.top = top; //Sets top to top
        pos.bottom = bottom; //Sets bottom to bottom
    }

    private: //Section that can't be accessed even by children
    struct brainPosition { //Struct to group the bounds of pixels into one name
        int left; //The left bound
        int right; //The right bound
        int top; //The top bound
        int bottom; //The bottom bound
    }; //The struct need not be global or accessible by children, just the variable made from it

    protected: //This section can't be accessed except by the children
    string name; //The name of the member
    brainPosition pos; //The position for it
};

class autons : public brainSpacing { //The autons class that inherits brainSpacing (parent class) and sets default to public
    public: //The public section

    autons(string auton_name, void (*Blue_Callback)(), void (*Red_Callback)()) { //Default constructor. Sets the name, and the 2 callbakcs
        name = auton_name; //Updates the name
        blueCallback = Blue_Callback; //Sets the blue callback
        redCallback = Red_Callback; //Sets the red callback
    } //Selected is false by default

    autons(string auton_name, void (*Blue_Callback)(), void (*Red_Callback)(), bool starts_selected) { //Constructor for setting a default selected auton
        name = auton_name; //Sets the name
        blueCallback = Blue_Callback; //Sets callback 1
        redCallback = Red_Callback; //Sets callback 2
        selected = starts_selected; //Sets the selected state to what is given
    }

    void (*blue_callback())() { //Returns the blue callback of the auton
        return blueCallback; //Returns it
    }

    void (*red_callback())() { //Returns the red callback of the auton
        return redCallback; //Returns it
    }

    bool isSelected() const { //Read-only for checking if the current auton is selected
        return selected; //Returns the current state
    }

    void setSelected(bool set_to) { //Sets the input boolean to the member's selected variable
        selected = set_to; //Updates the state
    }

    string nameIs() const { //Returns the name of the member's string
        return name; //Returns it
    }

    private: //The private section. The protected section of the parent gets added to this section
    void (*blueCallback)(); //Blue callback
    void (*redCallback)(); //Red callback
    bool selected = false; //Selected variable
};


class ManagerUtil : public brainSpacing { //The ManagerUtil structure for the top bar. brainSpacing is the parent
    public: //Public section

    ManagerUtil(string util_name, char ID) { //Constructor for setting the name and ID of the utility
        utilID = ID; //Sets the id
        name = util_name; //Sets the name
    }

    char getID() const { //Returns the id of the member utility
        return utilID; //Returns ID
    }

    int bottomPos() const { //Returns the bottom position of the member
        return pos.bottom; //Returns the integer
    }
    
    private: //The private section
    char utilID; //The ID of the member
};


class utilAutons : public brainSpacing { //Class that holds utility autons. Parent is brainSpacing
    public: //Public section

    utilAutons(string utilName, void (*singleCallback)()) { //The constructor for giving the name and callback
        name = utilName; //Sets the name
        callback = singleCallback; //Sets the callback
    } //Skills is assumed false by default

    utilAutons(string utilName, void (*singleCallback)(), bool is_skills) { //Alternative constructor for setting skills
        name = utilName; //Sets the name
        callback = singleCallback; //Sets the callback
        skills = is_skills; //Sets skills
    }

    void setSelected(bool state) { //Sets the selected state to the input provided
        selected = state; //Sets the state
    }

    bool isSelected() const { //Returns the current selected state of the member
        return selected; //Returns it
    }

    void (*callbackIs())() { //Returns the callback stored in the utilAutons Member
        return callback; //Returns the callback
    }

    string nameIs() const { //Returns the name of utilAutons member
        return name; //Returns the name
    }

    bool is_skills() const { //Returns the boolean of if it is skills
        return skills; //Returns the boolean
    }

    private: //Private section
    void (*callback)(); //The callback for the utilAuton
    bool selected = false; //The selected boolean
    bool skills = false; //Boolean for if it is skills. False by default
};

extern bool selectedIsBlue;
extern ez::Drive chassis; //The chassis for the bot

/*
 * This class does everything for the autonomous selector
 *  - Handles touches
 *  - Handles screen ID
 *  - Gets everything for the user
 *  - Generates utilities
 *  - Stores Util and Non-Util Autons
 *  - And More
*/
class AutonManager {
    public:

    //Template for the input autos (... allows for unspecified input size)
    template <typename... inputAutonTypes> //Allows for any amount of different autons to be placed into manager.
    void addAutons(inputAutonTypes... inputAutons) { //Function to add any autons or utilAutons class objects
        //I want to take the multiple types
        auto sendToVectors = [this] (auto inputAutonomous) { //Lambda to route the input autons to their vectors
            using T = std::decay_t<decltype(inputAutonomous)>; //Gets the type of the current auton

            if constexpr (std::is_same_v<T, autons>) { //If the type is autons
                autos.push_back(inputAutonomous); //Push to autos
            } else if constexpr (std::is_same_v<T, utilAutons>) { //If the type is utilAutons
                utilAutos.push_back(inputAutonomous); //Push to utilAutos
            }
        };

        (sendToVectors(inputAutons), ...); // Fold expression to call lambda for each inputAuton. Calls many time (... folds)
    }

    /*
     * Default Function for initializing manager.
     * - Defaults util bar height to 40 pixels
     * - Defaults row count to 2 rows
    */
    void initialize() {
        managerInit(40, 2);
    }

    /*
     * Alternative Overload for initializing manager
     * - Has an integer for row count
     * - Utility Bar height defaults to 40 pixels
    */
    void initialize(int autonRowCount) {
        managerInit(40, autonRowCount);
    }

    /*
     * Alternative Overload for initializing manager
     * - Has input of a double for utility height (converts to int for initializing)
     * - Rows defaults to 2
     * - Make sure to use the argument as a double, not an int
     *   * It converts the double to an int, truncating (rounds down)
    */
    void initialize(double utilityHeight) {
        managerInit(custom::num::convert<int>(utilityHeight), 2);
    }

    /*
     * Fully custom alternative overload for initializing manager
     * - utility height, and row count (ints)
     * - Allows for fully custom auton selector
    */
    void initialize(int autonRowCount, int utilityHeight) {
        managerInit(utilityHeight, autonRowCount); //Inits with custom parameters
    }

    int getID() const { //Returns the current screen ID
        return screenID; //Returns it
    }

    void setID(int ID) { //Sets the screen ID to input ID
        screenID = ID; //Sets it
    }

    void screenTouched(int x, int y) { //Checks everything for touch identification, then redraws screen

        if (debugLock.load()) throw 1; //If the debugLock is true (meaning the debug screen is being accessed), manager ignores touches

        if (utilities[3].containsPoint(x, y)) {
            autonomous();
            return;
        }

        for (auto &u : utilAutos) { //Checks the utility autons initially (check for auto debug menu run)
            if (u.nameIs() == "Debug Menu"&&u.containsPoint(x, y)) { //If one of the util autons is debug menu and was clicked
                void (*funcPointer)() = u.callbackIs(); //Store its callback in a new pointer
                funcPointer(); //Run the function
                funcPointer = nullptr; //Then free up the pointer (about to leave scope anyway)
                throw 1; //End screenTouched
            }
        }

        for (auto &u : utilities) { //Checks utilities
            if (u.containsPoint(x, y)) setID(u.getID()); //Updates ID if it should
        }

        isSkills = false; //Always starts false so on util screen it can set it, and it stays updated

        if (screenID != 3) { //If not 3 (utility screen)
            for (auto &u : utilAutos) { //Set all utilities to false
                u.setSelected(false); //Sets to false
            }

            for (auto &a : autos) { //Check the autons
                a.setSelected(a.containsPoint(x, y)); //Checks them
            }

        } else { //If the utility screen is up
            for (auto &a : autos) { //Set all autos to false
                a.setSelected(false); //Does that
            }

            for (auto &u : utilAutos) { //Check the utilities for touch
                u.setSelected(u.containsPoint(x, y)); //Checks them

                if (u.is_skills()&&u.isSelected()) isSkills = true; //If it is skills set it to true
            }

        }

        save_sd_card(); //Saves what is currently selected to the SD card
        drawScreen(); //Redraw after being done
    }

    void store() { //Updates the manager's internal pointer to what is selected
        bool autoWasSelected = false; //Tracker for if something is selected
        for (auto &a : autos) { //Goes through the autos
            if (a.isSelected()) { //If an auto is selected
                if (getID() == 1) { //Check the screen ID
                    storedCallback = a.blue_callback(); //Get blue if screen is blue
                    selectedIsBlue = true; //Updates the global variable
                } else if (getID() == 2) { //If screen is red
                    storedCallback = a.red_callback(); //Get the red callback
                    selectedIsBlue = false; //Updates the global variable
                }
                autoWasSelected = true; //Update the internal tracker
                autoIsSelected = true; //Update the global tracker (2 variable lock)
            }
        }

        for (auto &u : utilAutos) { //Then check the utility auton's
            if (u.isSelected()) { //If utility is selected
                storedCallback = u.callbackIs(); //Set internal pointer to the callback it has
                autoWasSelected = true; //Update the internal tracker
                autoIsSelected = true; //Update the global tracker
            }
        }

        if (!autoWasSelected) { //If no auton was selected (based on internal variables)
            storedCallback = nullptr; //Reset the pointer (lock 1)
            autoIsSelected = false; //Set global tracker to false (lock 2)
        }
    }

    void autonomous() { //Runs the stored autonomous routine
        if (storedCallback&&autoIsSelected) { //If stored callback != nullptr (lock 1) and global says something is selected (lock 2)
            storedCallback(); //Run the stored callback
        } else { //Otherwise
            basicDrive(); //Run the fallback auton
        } //Basic drive will also be added to the utility screen, but it acts as a default fallback
    }

    void drawBG() { //Draws the background of the brain screen
        drawImage(); //Function that draws the background
    }

    void terminateAutons() { //Prevents further brain printing.
        terminated = true; //Updates global tracker for termination
        utilities.clear(); //Clears the utilities
        autos.clear(); //Clears the autons
        utilAutos.clear(); //Clears the utility autons
        isInit = false; //Resets this (I don't plan on init and terminating the manager multiple times, but with this it can be done)
        drawBG(); //Redraws background after
    }

    bool hasTerminated() const { //Reads the current state of the termination variable
        return terminated; //Returns the variable
    }

    string selectedAuton() { //Returns the string name of the selected autonomous
        string output = ""; //String to store the output

        if (getID() != 3) { //If not the utility screen
            getID() == 1 ? output = "Blue" : output = "Red"; //Sets it to this by default (if nothing selected, show color)
            for (const auto &a : autos) { //Goes through the autons
                if (a.isSelected()) { //If any are selected
                    output = a.nameIs(); //Set the string to the name and
                    output += getID() == 1 ? " B" : " R"; //The color tag
                }
            }
        } else { //If it is the utility screen
            output = "Utility"; //Set it to this by default (same reason as before)
            for (const auto &u : utilAutos) { //Go through the utilAutons
                if (u.isSelected()) { //If one is selected
                    output = u.nameIs() + " U"; //Set the output to the name plus the U (Utility) tag.
                }
            }
        }

        return output; //Returns the output string
    }

    bool skills_is_selected() const { //Reads if the auton selected is skills
        return isSkills; //Returns the boolean
    }

    private: //Private Section of the manager
    vector<autons> autos; //Vector to store the autons
    vector<ManagerUtil> utilities; //Vector to place utilities (color and debug)
    unsigned short int screenID = 1; //1 by default
    void (*storedCallback)() = nullptr; //The callback storage
    bool autoIsSelected = true; //Global Tracker for autons. Auton does start selected by default
    bool terminated = false; //Tracker for if termination has occured
    vector<utilAutons> utilAutos; //Vector for the autons on the util screen
    bool isSkills = false; //Boolean to store if selected auton is skills
    bool isInit = false; //Boolean to store if the manager has been initialized
    atomic<bool> bgRedrawLock{true};

    static void touchRecog() {
        if (setupRecog) setupRecog->setupScreenRecognition();
    }

    void managerInit(int utilityHeight, int autonRowCount) { //Initializes the manager
        if (isInit) return; //Leaves early if already initialized

        instance = this; //Initializes the instance pointer to this
        setupRecog = this;

        for (int i = 1; i <= 4; ++i) { //Creates the utility sections. Can add more by adjusting numbers
            string name = ""; //Empty string for the name
            if (i == 1) name = "Blue"; //Blue has ID of 1
            if (i == 2) name = "Red"; //Red has ID of 2
            if (i == 3) name = "Utility"; //Utility has ID of 3
            if (i == 4) name = "Run Auto"; //A run auto button
            utilities.push_back(ManagerUtil(name, i)); //Adds them to utility
        }

        setupUtil(utilityHeight); //Sets the utility bar with input pixels of height
        setupMenu(utilities[0].bottomPos(), autonRowCount); //Sets up the autons starting at the utility bottom position

        drawBG(); //Draws the background (Bottom Layer)

        pros::delay(20); //Waits so brain can catch up

        load_sd_card_save(); //Loads the SD card (sets something to true)
        drawUtilBar(); //Draws utility bar from the function it's stored in
        drawScreen(); //Draws the screen after trying to load
        isInit = true; //Updates the initialized value to true when done all of this
        terminated = false; //Resets termination check
        setupScreenRecognition(); //Sets up screen recognition
        pros::Task newTask(touchRecog);
        pros::Task screenTask(screenStatic);
    }

    static inline AutonManager *setupRecog = nullptr;

    void setupUtil(int height) { //Sets up the utility buttons, based on input height
        int xPixel = 0; //Starting xPixel
        int xSlide = (480 - (utilities.size() * 8)) / utilities.size(); //Determines spacing by total amount
        for (auto &utils : utilities) { //For loop to set the utility's positions
            xPixel += 4; //Starts by moving forward 4 (spacing)
            int topBound = 0; //Sets top to 0
            int bottomBound = height; //Sets bottom to what is wanted
            int leftBound = xPixel; //Current pixel is left bound
            xPixel += xSlide; //Moves to the right however much is needed
            int rightBound = xPixel; //Sets the right bound to current pixel
            utils.setPosition(leftBound, rightBound, topBound, bottomBound); //Sets the positions
            xPixel += 4; //Spaces out once more
        } //It does this for every utility
    }

    struct overflowLimit { //A struct to group overflow booleans together (for the overflow system to work right)
        bool flag = false; //Flag for if already used (in for loops). Starts false
        bool check; //A boolean that is used to check for something. Is set in the program
    };

    void setupMenu(int minVertical, int vrows) { //Sets the auton's (& ManagerUtil) positions based on what order they were added. Does not draw the screen

        if (vrows < 1) { //This prevents divide by 0 errors and negative row counts (not possible)
            vrows = 1; //Brings to 1 if it is out of sizing
        }

        auto calculateRows = [&vrows] (size_t inputSize, vector<int>& output) { //Lambda to calculate rows and store them in inputted vector

            int totalCounter = 0; //Counter for how many total autons have been placed into rows

            int vrowsRestore = vrows; //The temporary variable to store vrows (so it can adapt and work across multiple calls)
            if (vrows > inputSize) { //When too many rows compared to autons needed to place
                vrows = inputSize; //Make how many rows are needed = to how many autons there are
            }

            int overflow = inputSize % vrows; //Variable to store over flow (when uneven compared to row count, returns remainder)
            atomic<bool> overflowDelay{true}; //An atomic to reduce lines used (vs bool) and to act as a lock
            int basicNum = inputSize / vrows; //The basic unmodified number for each row (overflow adds to this to get every auton)

            overflowLimit overflowCheck; //Variable that prevents overflow skip in systems with > 3 rows and that are
            //almost able to add a new auton to every row (makes second to last get the extra overflow, not top row)

            int magicNumber = vrows; //The magic number (The smallest multiple of rows that is bigger than the input size)
            while (magicNumber < inputSize) { //While the magic number is smaller than the input size (no = for safety for 1 row)
                magicNumber += vrows; //Adds to the magic number until it is bigger, acting as a multiple of rows
            }
            overflowCheck.check = (inputSize == magicNumber - 1); //Sets the check to if the input is one away from overflow 0

            if (vrows <= 3) overflowCheck.flag = true; //If too few for overflow to matter prevent fallthrough (for 3, 2, 1 rows)

            for (int i = 1; i <= vrows; i++) { //For loop to save how many autons go in each row (first row is bottom, last is top)
                if (i == 1) { //If first auton (will be last to get new auton)
                    output.push_back(basicNum); //Add the basic number
                } else if (i != vrows) { //If not the final row yet
                    if (overflow > 1&&(!overflowDelay.exchange(false)&&vrows > 3)) { //As long as overflow is not 1
                        //Final overflow will go to the last row. And if it is big enough to delay (and hasn't delayed)
                        output.push_back(basicNum + 1); //Add an overflow
                        --overflow; //And decrement overflow. I delay so it stacks upwards (example below)
                        //It moves the overflow up 1 in the "stack" changing how it works.
                        //Because of this for 4 rows (6 autons) it goes
                        // 1 2 1 2  ->  1 1 2 2         (This looks cleaner)
                    } else if (vrows == 3&&overflow > 1) { //If on three rows and there is overflow available
                        output.push_back(basicNum + 1); //Use that overflow in an auton
                        --overflow; //Count down overflow (this else if makes 1 1 3  ->  1 2 2   on the screen)
                    } else if (!overflowCheck.flag&&overflowCheck.check) { //If almost at overflow 0
                        overflowCheck.flag = true; //First row only will need an overflow, so this makes sure this can't activate
                        output.push_back(basicNum + 1); //Then it gives the overflow
                        --overflow; //And decrements as usual
                    } else output.push_back(basicNum); //If overflow is at limit (saved for last one) add usual number
                } else { //When it is the last row
                    output.push_back(inputSize - totalCounter); //Add the remaining autons needed to the last row
                }
                totalCounter += output.back(); //After each loop this gets updated with last addition, so that the final is correct
            }

            vrows = vrowsRestore; //Restores the vrow to what it was.
        };

        vector<int> autoRowCounts; //Vector to store how many autonomous rows are used (also has amount of autons per row)
        vector<int> utilRowCounts; //Vector to store amount of autons per row
        
        calculateRows(autos.size(), autoRowCounts); //Calculates the rows for the autons
        calculateRows(utilAutos.size(), utilRowCounts); //Calculates the rows for the utility autons

        //Assume 2 rows per screen
        //3 screens in total (red, blue, util)
        int horizontalPixel = 0; //Starts at left edge
        int verticalPixel = minVertical; //Sets the upper limit to the input provided

        //Before we can start setting the positions, we need to plan out distances
        //To do that, check how many autons there are (= amount on both screens)
        int bottomRowCount = autos.size() / 2; //Integer division, truncates
        int topRowCount = autos.size() - bottomRowCount; //Will always be the rest
        int ySize = 240 - minVertical; //Starting position of y tracker

        int autoni = 0; //Separate index for the autons themselves

        for (int i = autoRowCounts.size() - 1; i >= 0; i--) { //For loop for every row (first row is last in storage)
            for (int j = 1; j <= autoRowCounts[i]; j++) { //In every row, for amount of autons there should be, set their position
                horizontalPixel = setPosition(horizontalPixel, autoRowCounts[i], autos[autoni], verticalPixel, utilAutos[0], false, ySize, vrows);
                ++autoni; //Iterates autonomous index
            }

            horizontalPixel = 0; //Resets the horizontal pixel after each row
            verticalPixel += (240 - minVertical) / vrows; //Moves y pixel after each row
        }

        verticalPixel = minVertical; //For the utility row, reset the y position
        autoni = 0; //And reset the auton index

        for (int i = utilRowCounts.size() - 1; i >= 0; i--) { //For loop for every row (first is last in storage, so counts down)
            for (int j = 1; j <= utilRowCounts[i]; j++) { //Nested for loop for autons on each row
                horizontalPixel = setPosition(horizontalPixel, utilRowCounts[i], autos[0], verticalPixel, utilAutos[autoni], true, ySize, vrows);
                ++autoni; //Increments auton index
            }

            horizontalPixel = 0; //Resets x position after each row
            verticalPixel += (240 - minVertical) / vrows; //Moves y pixel after each row
        }
    }

    int setPosition(int pixel, int totalObjects, autons& currentAuto, int verticalPixel, utilAutons& utilAuto, bool usingUtil, int availableYPixels, int vertRows) { //Sets the input member's position
        pixel += 4; //Adds 4 for starting offset
        int leftBound = pixel; //What the left bound is
        int horizontalDistance = ((480 - (totalObjects * 8)) / totalObjects) / 2; //*8 is for spacing, then divide for the slide
        pixel += horizontalDistance; //Moves to the right edge
        int rightBound = pixel; //Right bound is now the current pixel
        pixel += 4; //Moves it right 4 again

        //Vertical Calculations
        verticalPixel += 4; //Offset
        int topBound = verticalPixel; //Top is now the current pixel
        int verticalDistance = (availableYPixels - (8 * vertRows)) / vertRows; //Decides based on rows wanted
        verticalPixel += verticalDistance; //Slides that distance
        int bottomBound = verticalPixel; //Creates bottom bound
        //No need to offset vertical pixel again, since the create menu does that automatically
        //It shifts the starting y for the bottom group before doing them

        if (!usingUtil) { //Checks the input boolean for what it should set. If setting the autons
            currentAuto.setPosition(leftBound, rightBound, topBound, bottomBound); //Sets the auto positioning
        } else { //If setting the utility
            utilAuto.setPosition(leftBound, rightBound, topBound, bottomBound); //Sets the util positioning
        }
        return pixel; //Returns pixel so it can stack
    }

    int redrawBGCounter = 0;

    void drawScreen() { //Draws the brain screen based on current ID and what's selected
        if (terminated) throw 1; //If already terminated, leave early.

        auto utilBarRedrawCheck = [this] () { //A check that will redraw the utility bar when it starts disappearing
            if (redrawBGCounter >= 48) drawUtilBar(); //Checks if above the threshold, and if so redraws
        };

        if (getID() == 3) { //If the screen is the utility screen

            bool expected = true; //This is what will make the background redraw, if the lock is == true
            bool desired = false; //This is what it will set the lock to when it is true (should redraw)

            if (bgRedrawLock.compare_exchange_strong(expected, desired)) { //This checks if it is true, and if so, sets it to false
                if (utilAutos.size() != autos.size()) {
                    drawBG(); //Then if it was the expected value, draw the background
                    pros::delay(20); //Delay
                    drawUtilBar(); //Redraw the utility bar (util bar doesn't need redraw besides when background is redrawn)
                    ++redrawBGCounter; //Count up the redraw counter (so when it comes time the utility bar redraws as well)
                }
            }

            utilBarRedrawCheck(); //Checks if it needs to redraw

            for (auto &u : utilAutos) { //Goes through every utility Auton
                textColor = WHITE; //Sets the text color to black
                color = u.isSelected() ? YELLOW : WHITE; //And the background to yellow if selected and white otherwise
                pros::screen::set_eraser(BLACK);
                u.drawBox(); //Then it draws the auton
            }
            //All screens auto-fill in any available space,
            //So even though the utilities probably will have less
            //With at least 2 it covers up the old screen when drawing, so no need to redraw background

        } else { //If any other screen

            bool expected = false; //This is what will make the background redraw, if the lock is == false
            bool desired = true; //This is what it will set the lock to when it is false (should redraw)

            if (bgRedrawLock.compare_exchange_strong(expected, desired)) { //For the same reason as explained above
                if (autos.size() != utilAutos.size()) {
                    drawBG(); //Draws background
                    pros::delay(20); //Refresh delay
                    drawUtilBar(); //Redraw utility bar
                    ++redrawBGCounter; //Increments the counter
                }
            } //This is the opposite of what is used for the utility check

            utilBarRedrawCheck(); //Checks if it needs to redraw

            for (auto &a : autos) { //Then it goes through every auton
                if (screenID == 1) textColor = BLUE; //If the screen is blue, set text to blue
                if (screenID == 2) textColor = RED; //If the screen is red, set text to red
            
                color = a.isSelected() ? YELLOW : BLACK; //Sets background color to yellow if selected, black if not
                pros::screen::set_eraser(BLACK);
                a.drawBox(); //Draws the auton
            }
        }
    }

    void drawUtilBar() { //Draws the utility bar to the screen
        for (auto &u : utilities) { //This draws the utilities
            if (u.getID() == 1) textColor = BLUE; //If blue, set text to blue
            if (u.getID() == 2) textColor = RED; //If red, set text to red
            if (u.getID() == 3) textColor = WHITE; //If utility, set text to white
            color = WHITE; //Background color is white
            pros::screen::set_eraser(BLACK);
            u.drawBox(); //Draws the utility box
        } //They get drawn once
    }

    void load_sd_card_save() { //Reads what's stored in the SD Card to load an auton
        try { //Attempts to check (if SD card not inserted this will fail, leading to the catch statement)
            string autonName = savedName(); //Reads the auton name
            char autonID = savedID(); //Reads the ID of the auton

            autoIsSelected = false; //Sets it to false by default

            setID(autonID); //Sets the screen ID so if nothing is selected the current screen is saved

            for (auto &a : autos) { //For every auton stored
                autonName == a.nameIs() ? a.setSelected(true) : a.setSelected(false);
                a.isSelected() ? autoIsSelected = true : 1; //If auto is selected, tell manager an auto is selected
            }

            for (auto &u : utilAutos) { //Then it looks through utilAutos
                if (autonName == u.nameIs()) { //If the correct name
                    bgRedrawLock.store(!bgRedrawLock.load()); //This makes the lock default to what it should be, correcting an issue
                    u.setSelected(true); //Set it to true
                    autoIsSelected = true; //Tells manager an auto is selected
                } else u.setSelected(false); //If not right auton set it to false
            }
        }
        catch (int error) { //If an exception is thrown, prevent crashes by using this
            switch (error) { //Checks the error code
                case 1: //My error I made
                printf("Error opening file"); //It prints to the console why
                break;
                default: //If not my own error
                printf("Unknown error handled"); //Prints that to console
                break;
            }
        }
    }

    void save_sd_card() { //Saves the current autonomous selected to the SD card
        try { //Try statement (to prevent crashes)
            save(); //Tries to save the auton
        }
        catch (int error) { //If it errored
            switch (error) { //Check the error code
                case 1: //If error code == 1 (my own custom)
                printf("Error opening file"); //Explain why
                break;
                default: //If not my own error
                printf("Unknown error handled"); //Print it to the console
                break;
            }
        }
    }

    void save() { //Saves the autonomous to the SD card
        ofstream autonFile("autonLog.txt"); //Opens the file (ofstream also creates the file)
        if (!autonFile.is_open()) throw 1; //If it isn't open, throw an exception
        string outputString = "\n" + std::to_string(getID()); //Sets it to the screen ID by default
        for (auto &a : autos) { //For every auton
            if (a.isSelected()) { //If it is selected
                outputString = a.nameIs() + "\n" + std::to_string(getID()); //Set the name, make a new line, add ID
            }
        }

        for (auto &u : utilAutos) { //Goes through the utilities
            if (u.isSelected()) { //If the utility is selected
                outputString = u.nameIs() + "\n3"; //Give it name and utility tag
            }
        }

        autonFile<<outputString<<endl; //Saves end result to the file
        autonFile.close(); //Closes the file to save it and clean up the output buffer
    }

    string savedName() { //Returns the name of the saved auton
        string name = ""; //string to put the name into
        ifstream autonFile("autonLog.txt"); //Opens the file
        if (!autonFile.is_open()) throw 1; //If not open, throw an exception
        int line = 1; //Line counter
        while (getline(autonFile, name)) { //Goes through every line
            if (line == 1) { //The name is stored at line 1
                cout<<name; //Save the line to the string
                break; //Leave the loop (we got what was wanted)
            }
            ++line; //Indexes line after the line check
        }
        autonFile.close(); //Closes the file to keep it clean
        return name; //Returns the fetched name
    }
    
    char savedID() { //Returns the ID of the selected's screen ID
        char id = 1; //Holds the id, 1 by default
        ifstream autonFile("autonLog.txt"); //Opens the file
        if (!autonFile.is_open()) throw 1; //If not open throw exception
        int line = 1; //Line counter
        string words; //String to store what the tag is
        while (getline(autonFile, words)) { //Goes through the file
            if (line == 2) { //At line 2
                cout<<words; //Save the tag to the words string
                break; //Leave the loop
            }
            ++line; //Index the line counter
        }
        if (words == "1") { //If it the tag is 1
            id = 1; //Set the id to blue (1)
        } else if (words == "2") { //If the tag is 2
            id = 2; //Set the id to red (2)
        } else { //Otherwise (when it would be 3)
            id = 3; //Set id to utility (3)
        }
        autonFile.close(); //Closes the file to keep it safe
        return id; //Returns the found id
    }

    atomic<bool> touchLock{false}; //Boolean for touches (locks it using atomics)
    static inline AutonManager *instance = nullptr; //A pointer to allow PROS to work properly

    void screenTouched() { //Runs when the screen is touched
        if (!touchLock.exchange(true)&&!terminated) { //When first entry on this screen touch and not terminated
            pros::screen_touch_status_s_t status = pros::screen::touch_status(); //Get the touch status
            screenTouched(status.x, status.y); //Give it to screenTouched
            master.print(0, 0, "AS: %-20s", selectedAuton()); //Print current auton to controller
            pros::delay(50); //Wait so rumble can que
            master.rumble("."); //Rumble the controller
            store(); //Store the current auton
        }
    }

    void screenReleased() { //When the screen is released
        touchLock.store(false); //Allow next screen touch to activate
    }

    static void touchStatic() { //A function to point to the other functions above
        if (instance) instance->screenTouched(); //Uses instance to call a this member through a static function
    }

    static void releaseStatic() { //The function to point to the released function
        if (instance) instance->screenReleased(); //Derefences the callback from this to static using instance
    }

    static void setup() {
        if (instance) instance->setupScreenRecognition();
    }

    static void timerRun() {
        if (instance) instance->timer(5);
    }

    void setupScreenRecognition() { //Updates the touch recognition
        pros::screen::touch_callback(touchStatic, pros::E_TOUCH_PRESSED); //Updates the pressed callback
        pros::screen::touch_callback(releaseStatic, pros::E_TOUCH_RELEASED); //Updates the pressed callback
        pros::Task setupTask(timerRun);
    }

    void timer(int minutes) {
        int timeElapsed = 0;
        double milliseconds = (minutes * 60 * 100);
        while (timeElapsed < milliseconds&&!terminated) {
            timeElapsed += 10000;
            pros::delay(10000);
        }
        if (!terminated) {pros::Task rerunSetup(setup);} //If terminated prevent infinite loop
    }

    static void screenStatic() { //The static function to run the background task loop
        if (instance) instance->backgroundTask();
    }

    static void backgroundTask() { //The background task to print xyt to screen
        double prevX = chassis.odom_x_get(); //Sets the previous x to current
        double prevY = chassis.odom_y_get(); //Sets the previous y to current
        double prevT = chassis.odom_theta_get(); //Sets previous theta to current
        int delayTime; //The delay time to change how long the loop is

        while (true) { //While the program is running

            delayTime = 80; //Delay is 80 milliseconds by default

            if (chassis.odom_x_get() > prevX + 0.25||chassis.odom_x_get() < prevX - 0.25) { //If out of tolerance
                pros::screen::set_pen(WHITE); //Set pen to white
                pros::screen::set_eraser(TRANSPARENT); //Set to transparent white
                pros::screen::print(pros::E_TEXT_MEDIUM, 260, 60, "X: %-10f", chassis.odom_x_get()); //Updates screen
                delayTime = 20; //Resfresh to next loop faster
                prevX = chassis.odom_x_get(); //Updates the previous x so it can detect next change
            }

            if (chassis.odom_y_get() > prevY + 0.25||chassis.odom_y_get() < prevY - 0.25) { //Out of tolerance
                pros::screen::set_pen(WHITE); //Set pen to white
                pros::screen::set_eraser(TRANSPARENT); //Set to transparent white
                pros::screen::print(pros::E_TEXT_MEDIUM, 260, 120, "Y: %-10f", chassis.odom_y_get()); //Update screen
                delayTime = 20; //Increase delay
                prevY = chassis.odom_y_get(); //Updates the previous y
            }

            if (chassis.odom_theta_get() > prevT + 0.25||chassis.odom_theta_get() < prevT - 0.25) { //Tolerance check
                pros::screen::set_pen(WHITE); //Set pen to white
                pros::screen::set_eraser(TRANSPARENT); //Set to transparent white
                pros::screen::print(pros::E_TEXT_MEDIUM, 260, 180, "Theta: %-10f", chassis.odom_theta_get()); //Update the screen
                delayTime = 20; //Increase delay
                prevT = chassis.odom_theta_get(); //Updates the previous theta
            }

            if (!chassis.drive_imu_calibrated()) break;

            pros::delay(delayTime); //Delay to save resources
        }

        while (true) {
            pros::screen::set_pen(WHITE); //Set pen to white
            pros::screen::set_eraser(TRANSPARENT); //Set to transparent white
            pros::screen::print(pros::E_TEXT_MEDIUM, 260, 60, "X: N/A"); //Updates screen
            pros::screen::print(pros::E_TEXT_MEDIUM, 260, 120, "Y: N/A"); //Update screen
            pros::screen::print(pros::E_TEXT_MEDIUM, 260, 180, "Theta: N/A"); //Update the screen

            pros::delay(120);
        }
    }
};

inline AutonManager AM; //Initialize and create the auton manager, AM
//If I haven't explained this elsewhere, inline is like extern. It allows for declaration across many files
//What's different is that inline allows for definition as well, due to some property in it having a lot of checks


#endif //End directive