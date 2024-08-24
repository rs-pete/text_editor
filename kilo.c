#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

struct termios org_termios;

void die(const char *s) { //check global error msg
    perror(s); //global error
    exit(1);
}

void disableRawMode() {
    if ( tcsetattr(STDIN_FILENO, TCSAFLUSH, &org_termios) == -1 ) //TCSAFLUSH helps to not run a bash for the unrecoqnised output
        die("tcsetattr");
}

void enableRawMode() {

    if( tcgetattr(STDIN_FILENO, &org_termios)  == -1 ) die("tcsetattr"); //temp save termios
    atexit(disableRawMode); //called at pgrm exit

    struct termios raw = org_termios;

    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_cflag |= (CS8);
    raw.c_oflag &= ~(OPOST);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN); //removes echoing chars before "q" und icanon = see tings we type
    raw.c_cc[VMIN] = 0; //min bytes before return
    raw.c_cc[VTIME] = 1; //max wait time for read; here 1/10 sec
    
      if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

int main() {
    enableRawMode();

    while (1) {
        char c = '\0';
            if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
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