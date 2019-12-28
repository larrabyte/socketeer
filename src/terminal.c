#include <stdio.h>

// Clears the terminal.
void clearterm() {
    printf("\033[2J");    // Moves cursor back to (0,0).
    printf("\033[1;1H");  // Removes remaining text.
}