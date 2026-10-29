#include "debug-mode.hpp"
#include "menu.hpp"

//The debug mode is designed to have some options on the screen (will probably use a class from menu to do that)
//That when pressed save debug info to the sd card in the file debug mode.
//It'll fully replace all data on the sd card in that debug file, instead of appending

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

//The function to clear up the screen and start the debug screen.
void initDebug(debugMenu menuSettings) {
    debugLock.store(true);

    std::vector<debugMenuItem> tempItems;

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
            horizontalPixel = setPosition(horizontalPixel, rowCounts[i], verticalPixel, ySize, vrows, tempItems[i]);
            ++autoni; //Iterates autonomous index
        }

        horizontalPixel = 0; //Resets the horizontal pixel after each row
        verticalPixel += (240 - minVertical) / vrows; //Moves y pixel after each row
    }

    debugLock.store(false);
}