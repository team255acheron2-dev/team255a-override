#include "Custom Extras/pidturncal.hpp" //The header file with the function declaration

struct step { //The structure to hold scores
    double offset; //The offset (delta)
    bool highStep; //The boolean for direction of step (used to keep locality)
};

bool readSDCard = false; //Variable if the sd card has been read (only want it to be once)

void readSDCardValue(double &P, double &I, double &D) { //Reads and updates the constants once.
    if (!readSDCard) { //If not turned off
        try { //Try for file safety
            ifstream file("pidResults.txt"); //Creates read-only file stream object
            if (!file.is_open()) throw 1; //If not open leave early
            std::string numberString = ""; //The string that stores the string of the values
            int desiredLineCounter = 0; //Counts how many lines in the total program (used to find the last and access it)
            while (std::getline(file, numberString)) { //Goes through 
                ++desiredLineCounter; //Counts how many lines are in the file (counts at \n)
                //It should end up equal to total lines - 1 (it counts \n, which the last line shouldn't have)
            }

            int lineCounter = 0; //The current line counter
            while (std::getline(file, numberString)) { //Goes through the file again
                ++lineCounter; //Updates the line counter

                if (lineCounter == desiredLineCounter - 1) { //Stop at last line (counts at \n, last line will be one less)
                    std::cout<<numberString; //Copy the line to the string
                    break; //Leave loop
                }
            }

            //Number string now contains the last line (most recent and hypothetical best trial)

            std::string currentNumber = ""; //The current number being found (stops at stopChar)
            std::string stopCharacter = ":"; //The character that shows when the number is complete
            short unsigned int constantIndex = 1; //The index for which PID constant to update
            for (int i = 0; i < numberString.size(); i++) { //Goes through the stored string
                if (numberString[i] != stopCharacter[0]) { //Weird way to say != ":"   idk char types are whacky
                    currentNumber += numberString[i]; //Adds current character
                } else { //When done with the value (reached :)
                    switch (constantIndex) { //Give the proper value to proper variable
                        case 1: //When first number
                        P = std::stod(currentNumber); //Give kP the number
                        break;
                        case 2: //Second number
                        I = std::stod(currentNumber); //Goes to kI
                        break;
                        case 3: //Third number
                        D = std::stod(currentNumber); //Goes to kD
                        break;
                    }
                    ++constantIndex; //Update constant tracker
                    currentNumber = ""; //Reset the number
                }
            }

            file.close(); //Close the file safely
        }
        catch (int e) { //If it fails
            printf("Please Insert SD Card"); //SD card not inserted so alert programmer
        }
    }
}

struct PIDpointerClass { //The class to hold PID pointers and a boolean if already set to the values
    double *P = nullptr; //The pointer to the kP value
    double *I = nullptr; //The pointer to the kI value
    double *D = nullptr; //The pointer to the kD value
    bool setToValues = false; //Boolean if the pointers are already set
};

PIDpointerClass PIDpointer; //The object that holds the static kP, kI, and kD values

float stepValue = 1; //The step value (hill-climb algorithm)
float pendingChange = 0; //Updates the values if they need to be updated

step runStep(int constantIndex) { //The function that does the step. Returns the better step

    static double kP = 11.0; //kP value (persists across calls)
    static double kI = 0.0; //kI value
    static double kD = 20.0; //kD value

    if (!PIDpointer.setToValues) { //Updates the pointers if they haven't been already
        PIDpointer.P = &kP; //Sets the kP pointer
        PIDpointer.I = &kI; //Sets the kI pointer
        PIDpointer.D = &kD; //Sets the kD pointer
        PIDpointer.setToValues = true; //And prevents this if from entering
    }

    readSDCardValue(kP, kI, kD); //Checks if it needs to update values

    switch (constantIndex) { //Updates based on better step (increase by step or decrease based on which was better)
        case 1: //If the index is at 1
        kP += pendingChange; //Increase by the offset dictated by the overall manager (below)
        break; //Leave (prevent fall-through)
        case 2: //If the index is at 2
        kI += pendingChange; //It will change kI
        break;
        case 3: //If the index is at 3
        kD += pendingChange; //Then it will update the kD
        break;
    }

    pendingChange = 0; //Resets the pending change

    double ogP = kP; //Sets the original kP
    double ogI = kI; //Original kI
    double ogD = kD; //Original kD

    bool upperStep = true; //Boolean to track which step was better

    auto findStep = [ogP, ogI, ogD] (double &modifyingValue, bool &stepDir) { //Lambda to run steps and return best trial
        chassis.odom_xyt_set(0_in, 0_in, 0_deg); //Reset the position
        double ogTheta = chassis.odom_theta_get(); //Get the original position

        modifyingValue += stepValue; //Increase a step

        chassis.pid_turn_constants_set(kP, kI, kD, 0.0); //Update the constants to everything stored (updates with step)

        chassis.pid_turn_set(90_deg, 90, false); //Turn 90 degrees at 90 speed without slew
        chassis.pid_wait(); //Wait until done

        double deltaT1 = std::abs(90 - (std::abs(util::wrap_angle(chassis.odom_theta_get()) - ogTheta))); //Find the delta distance (abs to compare)

        chassis.odom_theta_set(0_deg); //Resets the angle
        ogTheta = chassis.odom_theta_get(); //Update the original value

        modifyingValue -= (stepValue * 2); //Reduce down 2 steps (1st to bring back to original, 2nd to downstep)

        bool unnaturalZero = false;
        if (modifyingValue < 0) {modifyingValue = 0; unnaturalZero = true;} //If out of frame wrap back to 0, and update the bool

        chassis.pid_turn_constants_set(kP, kI, kD, 0.0); //Update the constants

        chassis.pid_turn_set(-90_deg, 90, false); //Turn -90 degrees (I will use abs later, but this way looks cleaner (doesn't do big circles))
        chassis.pid_wait(); //Wait until done

        double deltaT2 = std::abs(90 - (std::abs(util::wrap_angle(chassis.odom_theta_get()) - ogTheta))); //Find the offset

        chassis.pid_turn_constants_set(ogP, ogI, ogD, 0.0); //Resets the constants

        if (!unnaturalZero) modifyingValue += stepValue; //Reset the modifying value back to what it was (undo back step)
        //By using this check it only adds back the step if the original wasn't already at 0 (in the latter case it would wrap, so
        //this prevents it from accidentally adding a step when it shouldn't)

        if (deltaT1 <= deltaT2) {stepDir = true; return deltaT1;} //If upper was better return its results
        if (deltaT1 > deltaT2) {stepDir = false; return deltaT2;} //If lower was better return its results

        return 1000.00; //If neither was better (shouldn't be possible) return really big number to not mess with main loop
    };

    step newTrial; //The step object that will be returned

    switch (constantIndex) { //Based on the index use a different value
        case 1:
        newTrial.offset = findStep(kP, upperStep); //Runs the function with kP and sets offset to better step
        break;
        case 2:
        newTrial.offset = findStep(kI, upperStep); //Runs the function with kI and sets offset to better step
        break;
        case 3:
        newTrial.offset = findStep(kD, upperStep); //Runs the function with kD and sets offset to better step
        break;
    }

    newTrial.highStep = upperStep; //Updates the highstep to what the boolean was at the end

    return newTrial; //Return what the better trial was
}


void saveValues() { //Saves the current kP, kI, and kD values to the SD card
    std::ofstream pidFile("pidResults.txt", std::ios::app | std::ios::out); //Opens the file in writing mode (also creates if not created) and in appending
    if (!pidFile.is_open()) return; //If it isn't open end this early

    pidFile<<std::to_string(*PIDpointer.P)<<":" //Add the string version of pointer's dereferenced (kP)
           <<std::to_string(*PIDpointer.I)<<":" //Adds kI in string format with : separator
           <<std::to_string(*PIDpointer.D)<<":\n"; //Adds the last value and the new line (final ":" helps read function)

    pidFile.close(); //Close the file
}


void calibratePID() { //The function that runs all other functions related to calibrating the PID
    
    double deltaT0 = 180.00; //Starting initial offset (I init to a bigger number so it doesn't mess with the system)
    int counter = 0; //The counter (prevents getting stuck in the while loop forever)
    int indexCounter = 1; //The counter for the constant index (which kx to modify)
    int indexSubCounter = 0; //The subcounter to adjust the indexCounter
    step newDeltaT; //The newDelta step object
    int staticCounter = 0; //The counter for when improvement doesn't happen (change is static)
    while (deltaT0 > 0.5&&counter++ < 100) { //While the delta is greater than half a degree (delta0 is current values) and hasn't run 100 times
        ++indexSubCounter; //Increment the subcounter

        chassis.odom_xyt_set(0_in, 0_in, 0_deg); //Reset the robot's position
        double ogAngle = chassis.odom_theta_get(); //Get the original angle

        chassis.pid_turn_set(90_deg, 90, false); //Turn 90 degrees
        chassis.pid_wait(); //Wait until done

        deltaT0 = 90 - (std::abs(util::wrap_angle(chassis.odom_theta_get()) - ogAngle)); //Find the difference in angles (how off it was)

        if (indexSubCounter == 5) { //If 5 loops have happened
            indexSubCounter = 0; //Reset the subcounter
            indexCounter == 3 ? indexCounter = 1 : ++indexCounter; //If not at 3 (if so reset back to 1) increment the counter
        }

        newDeltaT = runStep(indexCounter); //Update the new delta to the step trials

        if (newDeltaT.offset < deltaT0) { //If the new offset is better than the current one
            if (newDeltaT.highStep) pendingChange += stepValue; //If it was the highstep update the pending change to increase
            else pendingChange -= stepValue; //If it was the low step make sure it lowers the step value
            staticCounter = 0; //Reset the static counter
        } else ++staticCounter; //If the new trial wasn't better then increment the static counter


        if (staticCounter > 2) { //If it is static for 3 or more runs (>2)
            stepValue /= 2; //Refine the steps (halve them)
            staticCounter = 0; //Reset the static counter
        }

        saveValues(); //Save the current values to the SD card
    }
}