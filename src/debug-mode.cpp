#include "debug-mode.hpp"
#include "menu.hpp"
#include <chrono>


void updateDebugItems() { //Refreshes (and creates) all the debug items
    //if (debugOptions.empty()) debugOptions.push_back(debugMenuItem("Filler"));
    std::vector<debugMenuItem> toggleSave = debugOptions; //Creates a copy of debugOptions to save the toggle state
    debugOptions.clear(); //Clears the whole vector to reset all data (so a fresh read can go through)

    auto restoreToggle = [toggleSave](debugMenuItem& item) { //A lambda to restore the toggle on the options
        for (auto &s : toggleSave) { //Goes through the copy
            if (s.nameIs() == item.nameIs()) { //If the name matches the given debugOption
                item.setToggle(s.isToggled()); //Sets the toggle to what was saved before
            }
        }
    };

    std::string xPos = std::to_string(chassis.odom_x_get()); //Gets the x position in a string
    std::string yPos = std::to_string(chassis.odom_y_get()); //Gets the y
    std::string direction = std::to_string(chassis.odom_theta_get()); //Gets the direciton
    debugOptions.push_back(debugMenuItem("Position", "X: " + xPos, "Y: " + yPos, //Saves all that data into the vector
                            "Direction: " + direction));
    //debugOptions.push_back(debugMenuItem("Port x Motor Temperature", ("Port x: ", x.whateverGetsTemp)));
    std::vector<std::string> loopVector;
    for (int i = 0; i < loopData.size(); i++) {
        for (int j = 0; j < loopData[i].size(); j++) {
            loopVector.push_back(loopData[i][j].exportData());
        }
    }
    debugOptions.push_back(debugMenuItem("Turn Cal Data", loopVector));

    debugOptions.push_back(debugMenuItem("Save")); //Adds the save button last (due to how I set up the screen last goes to bottom)

    for (int i = 0; i < debugOptions.size() - 1; i++) { //This restores all previous toggles
        if (toggleSave.empty()) break; //If nothing was stored before skip this (such as on first run of this function)
        restoreToggle(debugOptions[i]); //Restore the toggle for every element except the first (save button gets ignored)
    }
}

/* Current Version details
 *
 * This is a working build with all the features! It works as intended and does a very good job
 * 
 * As I use it I'll find things to patch and all that but nothing is really an issue
 * Do note that there is 1 observed issue.
 * 
 * Sometimes when the sd card is reinserted after being removed it fails to load properly
 * When that happens removing and inserting the sd card seems to fix it
 * I haven't consistently found what causes it but as time goes on I'll observe
 * Might just be the sd card though idk
*/

struct overflowLimit { //A struct to group overflow booleans together (for the overflow system to work right)
    bool flag = false; //Flag for if already used (in for loops). Starts false
    bool check; //A boolean that is used to check for something. Is set in the program
};

int setPosition(int pixel, int totalObjects, int verticalPixel, int availableYPixels, int vertRows, debugMenuItem& data) { //Sets the input member's position
    pixel += 4; //Adds 4 for starting offset
    int leftBound = pixel; //What the left bound is
    int horizontalDistance = ((480 - (totalObjects * 8)) / totalObjects); //*8 is for spacing, then divide for the slide
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

void saveDebugItems(std::vector<std::string> selectedItems) { //Saves the selected items to the sd card (so they automatically reselect)
    try { //Attempts to read file
        std::ofstream saveFile("DebugOptionsSave.txt"); //Creates and opens the file
        if (!saveFile.is_open()) throw 1; //If it isn't open (probably because no sd card) throw an error
        for (int i = 0; i < selectedItems.size(); i++) { //For all the selected items given
            saveFile<<(selectedItems[i] + "\n"); //Save each of them and some spacing into the file
        }
        saveFile.close(); //Close the save file when done
    } catch (int e) { //If it errors prevent program from ending
        printf("SD card not inserted\n"); //Print to console the likely cause
    }
}

void loadSavedItems() { //Loads all toggled items from previous session
    std::vector<std::string> toggledItems; //Vector to store all the toggled items' names
    try { //Attempts to open (error doesn't end program)
        std::ifstream saveFile("DebugOptionsSave.txt"); //Attempts to read the save file
        if (!saveFile.is_open()) throw 1; //If it isn't open (doesn't exist or no sd card) throw before errors
        std::string tempName; //The temporary name of each saved toggled item
        while (getline(saveFile, tempName)) { //Goes through all the lines of the file
            cout<<tempName; //Saves the line to tempName
            toggledItems.push_back(tempName); //Saves the item to the vector of names
            tempName = ""; //Resets the tempName variable
        }
        saveFile.close(); //Close when done
    } catch (int e) { //If it errors
        printf("SD card not inserted\n"); //Tell user probable cause
    }

    if (toggledItems.empty()) return; //If no items were added leave function
    for (int i = 0; i < toggledItems.size(); i++) { //For all the added names
        for (auto &d : debugOptions) { //Check all the debugOptions
            if (d.nameIs() == toggledItems[i]) { //If the name matches the stored name
                d.setToggle(true); //Set toggled
            }
        }
    }
}

std::atomic<bool> touchLock{false}; //The lock that makes sure only 1 touch goes through
bool screenRedraw = true; //The boolean to redraw background stuff for the first draw of the debug screen

void redrawDebug() { //Redraws the debug screen
    if (screenRedraw) { //If screen redraw allowed (on first instance)
        pros::screen::set_pen(0x0004FA); //A dark cyan/Mid-range blue
        pros::screen::fill_rect(0, 0, 480, 240); //Draws a rectangle over the whole screen
        pros::screen::set_pen(0xFFFFFF); //Set the text color to white
        pros::screen::print(pros::E_TEXT_LARGE, 0, std::int16_t(0), "Debug Mode"); //Prints text to let us know which screen is up
        screenRedraw = false; //Prevent the next background redraw
        loadSavedItems(); //Load any toggled items (only on the first draw before the items are drawn)
    }

    for (auto &d : debugOptions) { //For everything in debug options
        d.setTextColor(0xFFFFFF); //Set the text color of that item to white
        d.setBoxColor(d.isToggled() ? 0x00FA45 : 0xFFFFFF); //And set the box color to light cyan if toggled, white if not
        d.drawBoxUpdated(); //Draw the box using the corrected function in the class
    }
}

void touchChecks(int x, int y) { //The checks for normal touches (touches not on the save button)
    bool change = false; //Assumes no change was made by default
    for (auto &d : debugOptions) { //For everything in debugOptions
        if (d.containsPoint(x, y)) { //If the current item contains the point touched
            change = true; //Let us know something changed (so redraw happens)
            d.setToggle(!d.isToggled()); //Toggles the item
        }
    }

    if (change) redrawDebug(); //If a change happened, redraw the screen
}

std::atomic<bool> debugTurn{false}; //An atomic to control if the debug touch gets a turn (prevents this from happening during the
//transition back to AM's control)

void debugTouch(int x, int y) { //The function that checks for if the save button was pressed, and handles initial touches
    if (!touchLock.exchange(true)) { //If a touch hasn't already happened (exchange swaps values, and I do ! to check for false)

        if (!debugOptions.empty()&&debugOptions.back().containsPoint(x, y)) { //If the confirmation button was pressed (checks separately)
            
            std::vector<std::string> saveItems; //Create a vector of all the items saved
            for (auto &d : debugOptions) { //For every debug item
                if (!d.isToggled()) continue; //If not toggled, keep going
                saveItems.push_back(d.nameIs()); //Save the name of the item to the vector
            }
            saveDebugItems(saveItems); //Saves all toggled items to the sd card

            try { //Try to do some file shenanigans
                std::ofstream debugFile("debugLog.txt"); //Creates or opens the file. Not appending because no || [smth] app
                if (!debugFile.is_open()) throw 1; //If it isn't open (no sd card) throw to leave early.
                auto currentTime = std::chrono::system_clock::now(); //Gets the current time from the brain
                std::string heading = std::format("Debug Log - {:%Y-%m-%d %H:%M:%S} \n", currentTime); //Stores all that in the title for the log
                updateDebugItems(); //Update all the items before retrieving
                debugFile<<heading; //Saves the initial debug log info to the file
                for (size_t i = 0; i < debugOptions.size() - 1; ++i) { //Runs through everything except the first (save button)
                    if (!debugOptions[i].isToggled()) continue; //If the current data isn't toggled, keep going
                    debugFile<<debugOptions[i].getOutputData(); //Goes to each data and has it return what data to add
                    //It includes the name and all contents that it wanted.
                }

                debugFile.close(); //Close the file once done
            } catch(int e) { //If something is thrown, something went wrong
                printf("Error opening file. The sd card is probably not inserted."); //What probably went wrong. Prints to console
            }

            debugTurn.store(false); //Prevent the debug touch stuff from activating when going back to AM

            debugLock.store(false); //Once everything has saved and completed, open up the lock that was holding up the menu
            AM.reInit(); //Starts the menu selection back up
        } else touchChecks(x, y); //If the save button wasn't touched, do a general touch check
    }
}

void release() { //When the screen is released
    if (!debugTurn.load()) return; //If it is not the debug's turn for control leave this function
    touchLock.store(false); //Store false to let a new touch happen
}

void touchCallback() { //When the screen is touched
    if (!debugTurn.load()) return; //If not debug's turn, leave early
    pros::screen_touch_status_s_t status = pros::screen::touch_status(); //Get the screen status
    debugTouch(status.x, status.y); //Run the touch checks with the x and y of the touch
}

//The function to clear up the screen and start the debug screen.
void initDebug(debugMenu menuSettings) { //Initializes the debug everything, with input settings
    debugLock.store(true); //Turn on the global lock that prevents AM from interfering with the debug lock
    debugTurn.store(true); //Lets debug know it's turn is here
    updateDebugItems(); //Update the debug items
    screenRedraw = true; //Sets the boolean to true for first screen draw

    if (menuSettings.itemRows > debugOptions.size() - 1) menuSettings.itemRows = debugOptions.size() - 1;
    //If the provided rows exceeds how many items are there (-1 because save is separate), reduce it to max given
    int vrows = menuSettings.itemRows + 1; //How many rows there will be. How many are wanted + an extra space for the save button
    if (vrows <= 1) vrows = 2; //If the rows is below giving space for the save button or negative, put to lower limit

    auto calculateRows = [&vrows] (size_t inputSize, vector<int>& output) { //Lambda to calculate rows and store them in inputted vector

        int totalCounter = 0; //Counter for how many total autons have been placed into rows
        
        inputSize--; //Remove one from the input to pretend the save doesn't exist. I'll add it back in later

        int vrowsRestore = vrows; //The temporary variable to store vrows (so it can adapt and work across multiple calls)
        vrows--; //Remove one from the rows expected for calculations in here
        if (vrows > inputSize) { //When too many rows compared to autons needed to place
            vrows = inputSize; //Make how many rows are needed = to how many autons there are
        }

        int overflow = inputSize % (vrows); //Variable to store over flow (when uneven compared to row count, returns remainder)
        atomic<bool> overflowDelay{true}; //An atomic to reduce lines used (vs bool) and to act as a lock
        int basicNum = inputSize / (vrows); //The basic unmodified number for each row (overflow adds to this to get every auton)

        overflowLimit overflowCheck; //Variable that prevents overflow skip in systems with > 3 rows and that are
        //almost able to add a new auton to every row (makes second to last get the extra overflow, not top row)

        int magicNumber = (vrows); //The magic number (The smallest multiple of rows that is bigger than the input size)
        while (magicNumber < inputSize) { //While the magic number is smaller than the input size (no = for safety for 1 row)
            magicNumber += (vrows); //Adds to the magic number until it is bigger, acting as a multiple of rows
        }
        overflowCheck.check = (inputSize == magicNumber - 1); //Sets the check to if the input is one away from overflow 0

        if (vrows <= 3) overflowCheck.flag = true; //If too few for overflow to matter prevent fallthrough (for 3, 2, 1 rows)

        output.push_back(1); //Add in the save button slot we removed earlier (screen works first to last bottom to top)
        //Saves an extra row for the first element, which is guaranteed to be the save button
        //The screen assigns items and rows from top-bottom, with the first item being on bottom

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

    std::vector<int> rowCounts; //How many rows and how many items go in each row

    calculateRows(debugOptions.size(), rowCounts); //Calculates the rows from the items total size

    int minVertical = 30; //The vertical offset (gives space for the "Debug Menu" text)

    //Assume 2 rows per screen
    //3 screens in total (red, blue, util)
    int horizontalPixel = 0; //Starts at left edge
    int verticalPixel = minVertical; //Sets the upper limit to the input provided

    //Before we can start setting the positions, we need to plan out distances
    //To do that, check how many autons there are (= amount on both screens)
    int bottomRowCount = (debugOptions.size() - 1) / 2; //Integer division, truncates
    int topRowCount = debugOptions.size() - bottomRowCount; //Will always be the rest
    int ySize = 240 - minVertical; //Starting position of y tracker

    int autoni = 0; //Separate index for the autons themselves

    for (int i = rowCounts.size() - 1; i >= 0; i--) { //For loop for every row (first row is last in storage)
        for (int j = 1; j <= rowCounts[i]; j++) { //In every row, for amount of autons there should be, set their position
            horizontalPixel = setPosition(horizontalPixel, rowCounts[i], verticalPixel, ySize, vrows, debugOptions[autoni]);
            ++autoni; //Iterates autonomous index
        }

        horizontalPixel = 0; //Resets the horizontal pixel after each row
        verticalPixel += (240 - minVertical) / vrows; //Moves y pixel after each row
    }

    redrawDebug(); //Draws the debug screen

    pros::screen::touch_callback(touchCallback, pros::E_TOUCH_PRESSED); //Runs function on touch
    pros::screen::touch_callback(release, pros::E_TOUCH_RELEASED); //Runs the release function on release
}