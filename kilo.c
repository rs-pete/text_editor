#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

struct termios org_termios;

void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &org_termios); //TCSAFLUSH helps to not run a bash for the unrecoqnised output
}

void enableRawMode() {

    tcgetattr(STDIN_FILENO, &org_termios); //temp save termios

    atexit(disableRawMode); //called at pgrm exit

    struct termios raw = org_termios;

    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_cflag |= (CS8);
    raw.c_oflag &= ~(OPOST);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN); //removes echoing chars before "q" und icanon = see tings we type
    raw.c_cc[VMIN] = 0; //min bytes before return
    raw.c_cc[VTIME] = 1; //max wait time for read; here 1/10 sec
    
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {
    enableRawMode();

    while (1) {
        char c = '\0';
        read(STDIN_FILENO , &c , 1);
        if (iscntrl(c)) { //cortol char chk
            printf("%d\r\n", c);
        }
        else {
            printf("%d ('%c')\r\n", c, c); // 98('a')
        }
        if( c == 'q') break;
    }
    return 0;
}