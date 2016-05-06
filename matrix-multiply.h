#ifndef MATRIXMULTIPLY_H
#define MATRIXMULTIPLY_H

#include <pthread.h>
#include <stdexcept>
#include <QTextStream>
#include <QFile>
#include <QList>

// GLOBALS
    QFile infile1;
    QFile infile2;
    QFile outfile;
    QFile tempFile("temp.txt");

    QList< QList<int> > matrixA;
    QList< QList<int> > matrixB;
    //QList< QList<int> > matrixOut;

    int widthA;
    int widthB;
    int heightOut;
    int widthOut;

    struct position {
        int row;
        int col;
    };

    pthread_mutex_t lock;


// FUNCTIONS
    // remove temp.txt
    void remove_temp();
    // check for command line errors
    void check_cmd(int argc, char * argv[]);
    // spawn loader threads then join them
    void spawn_loader();
    // load files into list, threaded
    void * load_fxnA(void * param);
    void * load_fxnB(void * param);
    // check if matrix is valid
    void checkMatrix(QList< QList<int> > list);
    // check if matrices can be multiplied
    void compatible();
    // return the value of output at (row, col)
    int getValue(int row, int col);
    // create threads to calculate the values of the output
    void spawn_math();
    // called by thread
    void * write_temp(void * ptr);
    // parse temp.txt into matrixOut
    void parse_temp();

#endif // MATRIXMULTIPLY_H
