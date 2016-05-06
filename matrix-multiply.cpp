#include <QCoreApplication>

#include "matrix-multiply.h"

/*
  Sam McGhee
  ECE 3574
  Homework 6

  I used a thread joining synchronization method to ensure timing occured correctly.
  When reading the files, I spawned two threads, then waited for both to join before allowing my main thread to continue.
  I used a very similar method for ensuring the multiplication was synchronized properly.

  When making sure that no threads had a race conditition on the the temp.txt file, I used a simple mutex lock.
 */

QTextStream qCout(stdout);
QTextStream qErr(stderr);

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    // Check the command line arguments, files exist
    check_cmd(argc,argv);
    // remove temp .txt
    remove_temp();
    // start 2 threads to load the files into lists
    spawn_loader();
    // make sure matrices are compatible
    compatible();
    // start threads to perform the multiplication
    spawn_math();
    // parse the temp.txt into a matrix then to outfile
    parse_temp();
    // delete temp.txt
    remove_temp();

    return EXIT_SUCCESS;
}

void spawn_math()
{
    pthread_t thread[heightOut*widthOut];
    int rc;
    long status;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    int t = 0;
    position * pos;

    pthread_mutex_init(&lock, NULL);

    for (int row = 0; row<heightOut; row++)
    {
        for (int col=0; col<widthOut; col++)
        {
            pos = (position *)malloc(sizeof(pos));
            pos->row = row;
            pos->col = col;

            rc = pthread_create(&thread[t], &attr, write_temp, (void *)pos);
            if (rc)
            {
                qErr << "failed to create thread" << endl;
                exit(1);
            }
            t++;
        }
    }

    // join threads
    pthread_attr_destroy(&attr);
    for (int i = 0; i<t; i++)
    {
        rc = pthread_join(thread[i], (void **)&status);
        if (rc)
        {
            qErr << "failed to join threads" << endl;
            exit(1);
        }
    }
}

void * write_temp(void  * ptr)
{
    position * pos = (position * )ptr;
    int value = getValue(pos->row,pos->col);

    // write the value to the temp.txt file
    QString line = QString("C%1,%2=%3").arg(pos->row).arg(pos->col).arg(value);
    // lock on temp.txt
    pthread_mutex_lock(&lock);

    tempFile.open(QIODevice::Append);
    QTextStream out(&tempFile);
    out << line << endl;
    tempFile.close();

    pthread_mutex_unlock(&lock);

    pthread_exit(NULL);
}

void parse_temp()
{
    QString temp, temp1, temp2, temp3;
    bool ok;
    QStringList listval;
    int row, col, val;

    tempFile.open(QIODevice::ReadOnly);
    QTextStream in(&tempFile);

    int matrixOut[heightOut][widthOut];

    do {
            temp = in.readLine();
            if (temp.length() > 0)
            {
                // split the line into a QList<int>
                // Crow,col=V
                temp.remove("C");
                listval = temp.split("="); // {x,y  ,v}

                temp1 = listval.at(1);
                val = temp1.toInt(&ok,10);
                if (!ok)
                {
                    qErr << "could not convert temp.txt val to int" << endl;
                    exit(1);
                }
                temp1 = listval.at(0);
                listval = temp1.split(",");

                temp2 = listval.at(0);
                row = temp2.toInt(&ok, 10);
                if (!ok)
                {
                    qErr << "could not convert temp.txt row to int" << endl;
                    exit(1);
                }
                temp3 = listval.at(1);
                col = temp3.toInt(&ok, 10);
                if (!ok)
                {
                    qErr << "could not convert temp.txt col to int" << endl;
                    exit(1);
                }

                // Store values to matrix
                matrixOut[row][col] = val;
            }
            listval.clear();
        } while (!temp.isNull());

    tempFile.close();

    outfile.open(QIODevice::WriteOnly);
    QTextStream out(&outfile);

    for (int i=0; i<heightOut; i++)
    {
        for (int j=0; j<widthOut; j++)
        {
            out << matrixOut[i][j] << " ";
        }
        out << endl;
    }

    outfile.close();
}

void checkMatrix(QList< QList<int> > list)
{
    int current = 0, prev = 0;
    bool first = true;

    for (int i=0; i<list.length(); i++)
    {
        if (first)
        {
            current = list.at(i).length();
        }
        else
        {
            prev = current;
            current = list.at(i).length();
            if (prev != current)
            {
                qErr << "Invalid Matrix file" << endl;
                exit(1);
            }
        }
    }
}


void spawn_loader()
{
    pthread_t pids[2];
    int rc;
    long status;
    pthread_attr_t attr;
    /* Initialize and set thread detached attribute */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    // Generate threads
    rc = pthread_create(&pids[0], &attr, load_fxnA, NULL);
    if (rc)
    {
        qErr << "failed to create thread" << endl;
        exit(1);
    }

    rc = pthread_create(&pids[1], &attr, load_fxnB, NULL);
    if (rc)
    {
        qErr << "failed to create thread" << endl;
        exit(1);
    }

    // Free attribute and wait for the other threads
    pthread_attr_destroy(&attr);
    for (int i = 0; i<2; i++)
    {
        rc = pthread_join(pids[i], (void **)&status);
        if (rc)
        {
            qErr << "failed to join threads" << endl;
            exit(1);
        }
    }
}


void * load_fxnA(void * param)
{
    QString temp;
    infile1.open(QIODevice::ReadOnly);
    QTextStream in(&infile1);
    QStringList tempList;
    QList<int> intList;
    int toAdd;
    bool ok;

    do {
            temp = in.readLine();
            if (temp.length() > 0)
            {
                if (temp.contains("\t"))
                    tempList = temp.split("\t");
                else
                    tempList = temp.split(" ");
                for (int i=0; i<tempList.length(); i++)
                {
                    temp = tempList.at(i);
                    toAdd = temp.toInt(&ok,10);
                    if (!ok)
                    {
                        qErr << "could not convert to int (load fxnA)" << endl;
                        exit(1);
                    }
                    intList.append(toAdd);
                }
                matrixA.append(intList);
            }
            tempList.clear();
            intList.clear();
        } while (!temp.isNull());

    infile1.close();
    checkMatrix(matrixA);
    pthread_exit(NULL);
}

void * load_fxnB(void * param)
{
    QString temp;
    infile2.open(QIODevice::ReadOnly);
    QTextStream in(&infile2);
    QStringList tempList;
    QList<int> intList;
    int toAdd;
    bool ok;

    do {
            temp = in.readLine();
            if (temp.length() > 0)
            {
                if (temp.contains("\t"))
                    tempList = temp.split("\t");
                else
                    tempList = temp.split(" ");
                for (int i=0;i<tempList.length();i++)
                {
                    temp = tempList.at(i);
                    toAdd = temp.toInt(&ok,10);
                    if (!ok)
                    {
                        qErr << "could not convert to int (load fxnB)" << endl;
                        exit(1);
                    }
                    intList.append(toAdd);
                }
                matrixB.append(intList);
            }
            tempList.clear();
            intList.clear();
        } while (!temp.isNull());

    infile2.close();
    checkMatrix(matrixB);
    pthread_exit(NULL);
}

int getValue(int row, int col)
{
    int sum = 0;
    for (int i=0; i<widthA; i++)
    {
        sum += (matrixA.at(row).at(i) * matrixB.at(i).at(col));
    }
    return sum;
}

void compatible()
{
    widthA = matrixA.at(0).length();
    widthB = matrixB.at(0).length();

    heightOut = matrixA.length();
    widthOut = widthB;

    if (widthB != matrixA.length())
    {
        qErr << "matrices cannot be multiplied" << endl;
    }
}

void check_cmd(int argc, char * argv[]) {

    if (argc != 4)
    {
        qErr << "Not enough arguments specified" << endl;
        exit(1);
    }

    infile1.setFileName(argv[1]);
    infile2.setFileName(argv[2]);
    outfile.setFileName(argv[3]);

    if (!infile1.exists())
    {
        qErr << "Infile: " << infile1.fileName() << " does not exist" << endl;
        exit(1);
    }
    if (!infile2.exists())
    {
        qErr << "Infile: " << infile2.fileName() << " does not exist" << endl;
        exit(1);
    }
}

void remove_temp()
{
    if (tempFile.exists())
        tempFile.remove();
}
