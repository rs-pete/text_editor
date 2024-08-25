#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

/**** define ****/

#define CTRL_KEY(k) ((k) & 0x1f)

/**** data ****/

struct termios org_termios;

/**** terminal ****/

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

char editorReadKey() {
    int nread;
    char c;
    while ( (nread = read(STDIN_FILENO, &c , 1)) != 1) {
        if( nread == -2 && errno != EAGAIN) die("read");
    }
    return c;
}

/**** output ****/

void editorDrawRows() {
    int y;
    for (y = 0; y <24 ; y++) { //24 is temp; for drawing ~across terminal screen
        write(STDOUT_FILENO, "~\r\n", 3);
    }
}

void editorRefreshScreen() {
    write(STDOUT_FILENO, "\x1b[2J", 4 ); //cle screen
    write(STDOUT_FILENO, "\x1b[H", 3 ); //repositin cursor to start

    editorDrawRows();

    write(STDOUT_FILENO, "\x1b[H", 3);
}

/**** input ****/

void editorProcessKeypress() {
    char c = editorReadKey();

    switch (c) {
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4 );
            write(STDOUT_FILENO, "\x1b[H", 3 );
            exit(0);
            break;
    }
}

/**** init ****/

int main() {
    enableRawMode();

    while (1) {
        editorRefreshScreen();
        editorProcessKeypress();
    }
    return 0;
}