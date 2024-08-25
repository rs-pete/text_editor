#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <sys/ioctl.h>

/**** define ****/

#define CTRL_KEY(k) ((k) & 0x1f)

/**** data ****/

struct editorConfig { //global var creation
    int screenrows;
    int screencols; 
    struct termios org_termios;
};

struct editorConfig E;

/**** terminal ****/

void die(const char *s) { //check global error msg
    perror(s); //global error
    exit(1);
}

void disableRawMode() {
    if ( tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.org_termios) == -1 ) //TCSAFLUSH helps to not run a bash for the unrecoqnised output
        die("tcsetattr");
}

void enableRawMode() {

    if( tcgetattr(STDIN_FILENO, &E.org_termios)  == -1 ) die("tcsetattr"); //temp save termios
    atexit(disableRawMode); //called at pgrm exit

    struct termios raw = E.org_termios;

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

int getCursorPosition( int *rows, int *cols) {
    char buf[32];
    unsigned int i = 0;

    if (write(STDOUT_FILENO, "\x1b[6n" , 4 ) != 4) return -1;

    while ( i < sizeof( buf ) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    
    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[') return -1;
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;

    return 0;
}
int getWindowSize(int *rows, int *cols) {
    struct winsize ws;

    //error check
    // 0 is a possible error outcome; idk why
    if (1 || ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) { 
        //fallbak if iocntl wont work
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
        return getCursorPosition(rows, cols);
    } 
    else {
        //terminal widith and height
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

/**** output ****/

void editorDrawRows() {
    int y;
    // draw ~ across terminal size like vim
    // change 24 to actual termial width
    for (y = 0; y < E.screenrows ; y++) {
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

void initEditor() {
    if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
}

int main() {
    enableRawMode();
    initEditor();

    while (1) {
        editorRefreshScreen();
        editorProcessKeypress();
    }
    return 0;
}