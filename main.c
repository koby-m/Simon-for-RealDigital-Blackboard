/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

        Program Name:           Simon Game

        Contributor(s):         Koby Miller
        Date last modified:     February 1st, 2024

        Description:            A remake of the Simon game where the player repeats back an increasingly complex pattern indicated via
                                flashing lights. On-board buttons control the game, with score and messages shown on the on-board
                                seven-segment display.
                                
                                Has three toggleable features: 
                                        - Only-newest mode: Does not repeat the entire pattern back, only the latest addition
                                        - Color mode: Will utilize the RGB LEDs to play back the pattern
                                        - Debug mode: Will make the pattern only consist of one number

+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */

/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  
        INCLUDE

+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
#include "klib.h"
#include <stdlib.h>
#include <sys/_intsup.h>
#include <sys/unistd.h>

/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  
        MACROS

+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
#define PATTERN_SIZE 40 // how big of a pattern should generate
#define PATTERN_DIGITS 4 // how many different numbers could appear

#define FLASH_TIME 1/(iter * 10000) + 100000 // time between flashing lights

#define SINGLE_SWITCH 0 // switch to indicate only flash newest addition to pattern
#define COLOR_SWITCH 1 // switch to indicate color mode
#define DEBUG_SWITCH 2 // switch to indicate debug mode (only one position is repeated)

/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  
        PROTOTYPES

+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
void lightUp(unsigned int data);
void lightsOff();

/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  
        MAIN
        Last modified: February 1st, 2024

+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
int main() {

    unsigned int seed; //seed for srand(), left uninitialized for more 'random' value
    unsigned int pattern[PATTERN_SIZE]; //array of randomly generated pattern

    unsigned int iter; // game round iterator
    unsigned int lose; // true/false to continue the game
    unsigned int c; // for loop iterator

    enableRGB(0b11); //enable both RGB LEDs

    while(1) {
        // start
        enableSevSeg(1);
        outputToSevSegCustom("PLAY");
        outputToRGB(0,0x00FF00,0.1);
        outputToRGB(1,0x00FF00,0.1);

        // wait for user button        
        // while waiting increment the 'seed' variable so that it is 'random'
        while(getButtonStates() == 0) {
            seed++;
        }
        
        if(bitIndex(getSwitchStates(),COLOR_SWITCH) == 1) {  // if in multiple color mode is on
            // turn off LEDs
            outputToRGB(0,0,0);
            outputToRGB(1,0,0);


            // demo button colors
            usleep(500000);
            lightUp(0b0001);

            usleep(500000);
            lightUp(0b0010);

            usleep(500000);
            lightsOff();
            lightUp(0b0100);

            usleep(500000);
            lightUp(0b1000);

            usleep(500000);
            lightsOff();
        }

        usleep(100000);

        // seed with random time from waiting loop
        srand(seed);

        // fill pattern matrix with random values
        for(c = 0;c < PATTERN_SIZE;c++) {
            
            if( bitIndex(getSwitchStates(), DEBUG_SWITCH) != 1 ) { //default mode

                pattern[c] = (unsigned int)(1 << (rand() % PATTERN_DIGITS));
            
            } else { // testing mode if debug switch is on, fill pattern with 1s

                pattern[c] = 1;

            }

        }

        // reset iterators
        iter = 1;

        // reset loss state
        lose = 0;

        // set sev-seg to display score
        enableSevSeg(0); // number mode
        outputToSevSeg(0, 1); 

        // game start
        do {
            // small pause
            sleep(1);

            // show score, which is based on number of correct patterns completed: iter
            outputToSevSeg((iter - 1) * 10, 1);
            
            //color yellow during playback, if not in color mode
            if(bitIndex(getSwitchStates(), COLOR_SWITCH) != 1) {
                outputToRGB(0,0xff4d00,0.1);
                outputToRGB(1,0xff4d00,0.1);
            }        
            
            // play pattern
            for(c = 0;c < iter;c++) {
                
                // if SINGLE_SWITCH switch is ON then only play back newest part of the pattern (flash once)
                if( bitIndex(getSwitchStates(),SINGLE_SWITCH) ) {
                    c = iter - 1;
                }
                
                // write to LEDs, flashing
                lightUp(pattern[c]);
                usleep(FLASH_TIME);
                lightsOff();
                usleep(FLASH_TIME);
            }
            
            // blue lights if it is the players turn and color mode is off
            if( bitIndex(getSwitchStates(), COLOR_SWITCH) != 1 ) {
                outputToRGB(0,0x00F0FF,0.1);
                outputToRGB(1,0x00F0FF,0.1);
            }
            
            // user repeats pattern
            for(c = 0;c < iter;c++) {
                
                // wait for user input
                while(getButtonStates() == 0);

                // light up the pressed button
                lightUp(getButtonStates());
                
                // compare button presses with pattern
                if(getButtonStates() != pattern[c]) {
                    c = iter; // leave for loop
                    lose = 1; // set loss status
                }
                
                // wait for button release
                while(getButtonStates() != 0);

                // turn off LEDs
                lightsOff();
                
                usleep(100000);

            }

            // increase how much of the pattern should be repeated
            iter++;
            
        } while (lose != 1); // check to see if the player got something incorrect
        
        // player loses
        // switch sev-seg to show message
        enableSevSeg(1);

        // "YOU LOSE"
        outputToRGB(0,0xFF0000,0.1);
        outputToRGB(1,0xFF0000,0.1);
        for(c = 0; c < 3;c++) {
            outputToSevSegCustom("YOU ");
            sleep(1);
            outputToSevSegCustom("LOSE");
            sleep(1);
        }
    }
    
    return 0;
}//end main()

/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  
        DEFINITIONS

+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        lightUp();
		
        unsigned int data       Pattern data to display on either LEDs or RGB LEDs based
                                on whether 'color mode' is on or not (Checks switch index 
                                of COLOR_SWITCH) 

            lightUp(0b0010)     Color mode: will light up 1st RGB LED green
                                Normal mode: will light up 2nd LED
                                
            lightUp(0b0001)     Color mode: will light up 1st RGB LED red
                                Normal mode: will light up 1st LED
            
        Written by Koby Miller
        Last modified: February 1st, 2024
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
void lightUp(unsigned int data) {
    if(bitIndex(getSwitchStates(), COLOR_SWITCH) != 1) { // normal mode is on

        outputToLEDs(data);

    } else { // color mode is on

        switch(data) {
                case 0b0001:
                    outputToRGB(0, 0xFF0000, 0.1); //red
                    break;

                case 0b0010:
                    outputToRGB(0, 0x00FF00, 0.1); //green
                    break;

                case 0b0100:
                    outputToRGB(1, 0x0000FF, 0.1); //blue
                    break;

                case 0b1000:
                    outputToRGB(1, 0xFF4D00, 0.1); //yellow
                    break;
                    
                default:
                    break;
            }
    }
   
    
    return;
}

/* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        lightsOff();

            Turns off all LEDs corresponding if color mode is on/off
            
        Written by Koby Miller
        Last modified: February 1st, 2024
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */
void lightsOff() {
    if(bitIndex(getSwitchStates(), COLOR_SWITCH) != 1) { // normal mode is on

        outputToLEDs(0);

    } else { // color mode is on

            outputToRGB(0, 0, 0); // 1st RGB LED

            outputToRGB(1, 0, 0); // 2nd RGB LED
    }

    return;
}