#include "debug-mode.hpp"
#include "menu.hpp"
#include <chrono>

//The debug mode is designed to have some options on the screen (will probably use a class from menu to do that)
//That when pressed save debug info to the sd card in the file debug mode.
//It'll fully replace all data on the sd card in that debug file, instead of appending

/* Current Version details
 *
 * The debug-mode.hpp file is mostly done. There's not a lot besides customization for the debugMenu struct that needs to be done
 * main.cpp and menu.hpp were changed to allow for AM (AutonManager) to be referenced here, mainly to start it back up
 * This file still needs to be worked on a lot more. I will have to decide on where to have the system get all the details
 * to print in the debug. Where do I have that stored/called and the class for debugMenuItem might need to be changed
 * to allow for the data to be updated (depending on where the class objects are made will determine that)
 * 
 * For the next time I need to code, here is my current agenda list:
 *  Create where the vector that stores debugMenuItem objects is given info
 *  Update the debugTouch function to check screen touches for toggling
 *  Update the row calculation to add the last item of menuItems to a new row
 *  And the boxes still have to be drawn. That code needs to be added all around
 *  Remember to add in the system to restart up the menu processes (might need to add new function to AM to do that)
 *  Note: menuItems[last] is the button to save all selected data
*/

struct overflowLimit { //A struct to group overflow booleans together (for the overflow system to work right)
    bool flag = false; //Flag for if already used (in for loops). Starts false
    bool check; //A boolean that is used to check for something. Is set in the program
};

int setPosition(int pixel, int totalObjects, int verticalPixel, int availableYPixels, int vertRows, debugMenuItem& data) { //Sets the input member's position
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

    data.setPosition(leftBound, rightBound, topBound, bottomBound);
    return pixel; //Returns pixel so it can stack
}

std::atomic<bool> touchLock{false};

std::vector<debugMenuItem> menuItems;

void debugTouch(int x, int y) {
    if (!touchLock.exchange(true)) { //If a touch hasn't already happened (exchange swaps values, and I do ! to check for false)

        if (menuItems.back().containsPoint(x, y)) { //If the confirmation button was pressed (checks separately)
            auto currentTime = std::chrono::system_clock::now(); //Gets the current time from the brain
            std::string outputTextStream = std::format("Debug Log - {:%Y-%m-%d %H:%M:%S} \n", currentTime); //Stores all that in the title for the log
            for (int i = 0; i < menuItems.size() - 1; i++) { //Runs through everything except the last (save button)
                if (!menuItems[i].isToggled()) continue; //If the current data isn't toggled, keep going
                outputTextStream += menuItems[i].getOutputData(); //Goes to each data and has it return what data to add
                //It includes the name and all contents that it wanted.
            }

            try {
                ofstream debugFile("debugLog.txt"); //Creates or opens the file. Not appending because no || [smth] app
                if (!debugFile.is_open()) throw 1; //If it isn't open (no sd card) throw to leave early.
                debugFile<<outputTextStream<<endl; //Saves the completed string file to that file
                debugFile.close();
            } catch(int e) { //If something is thrown, something went wrong
                printf("Error opening file. The sd card is probably not inserted."); //What probably went wrong. Prints to console
            }

            debugLock.store(false); //Once everything has saved and completed, open up the lock that was holding up the menu
        }
    }
}

void release() {
    touchLock.store(false);
}

//The function to clear up the screen and start the debug screen.
void initDebug(debugMenu menuSettings) {
    debugLock.store(true);

    pros::screen::set_pen(0x0004FA); //A dark cyan/Mid-range blue
    pros::screen::fill_rect(0, 0, 480, 240); //Draws a rectangle over the whole screen
    std::string titleText = "Debug Mode";
    pros::screen::print(pros::E_TEXT_MEDIUM, 0, std::int16_t(0), "Debug Mode");

    int vrows = menuSettings.rows;

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

    //The lambda is copied from the menu.hpp system, since that is already set in place.

    std::vector<int> rowCounts;

    calculateRows(menuSettings.saveItems.size(), rowCounts);

    int minVertical = 20;

    //Assume 2 rows per screen
    //3 screens in total (red, blue, util)
    int horizontalPixel = 0; //Starts at left edge
    int verticalPixel = minVertical; //Sets the upper limit to the input provided

    //Before we can start setting the positions, we need to plan out distances
    //To do that, check how many autons there are (= amount on both screens)
    int bottomRowCount = menuSettings.saveItems.size() / 2; //Integer division, truncates
    int topRowCount = menuSettings.saveItems.size() - bottomRowCount; //Will always be the rest
    int ySize = 240 - minVertical; //Starting position of y tracker

    int autoni = 0; //Separate index for the autons themselves

    for (int i = rowCounts.size() - 1; i >= 0; i--) { //For loop for every row (first row is last in storage)
        for (int j = 1; j <= rowCounts[i]; j++) { //In every row, for amount of autons there should be, set their position
            horizontalPixel = setPosition(horizontalPixel, rowCounts[i], verticalPixel, ySize, vrows, menuItems[i]);
            ++autoni; //Iterates autonomous index
        }

        horizontalPixel = 0; //Resets the horizontal pixel after each row
        verticalPixel += (240 - minVertical) / vrows; //Moves y pixel after each row
    }

    pros::screen::touch_callback([]() {pros::screen_touch_status_s_t status = pros::screen::touch_status(); //Lambda to combine into one function
                                       debugTouch(status.x, status.y);}, pros::E_TOUCH_PRESSED); //Gets touch info and runs function
    pros::screen::touch_callback(release, pros::E_TOUCH_RELEASED);
}