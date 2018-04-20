#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

 
int sum(int arr[], int n)
{
    int i;
    int sum = 0; // initialize sum
   
    for (i = 0; i < n; i++){
        sum += arr[i];
    }
   
    return sum;
}
 
int *countMoves(int **table, int currRow, int currCol, int m, int n, int *arr){
   
    //2 right 1 up
    if((currCol + 2 < n && currRow - 1 >= 0) && table[currRow-1][currCol+2] == 0){
        arr[0] = 1;
    }
    else{
        arr[0] = 0;
    }
   
    //2 right 1 down
    if((currCol + 2 < n && currRow + 1 < m) && table[currRow+1][currCol+2] == 0){
        arr[1] = 1;
    }
    else{
        arr[1] = 0;
    }
   
    //2 left 1 up
    if((currCol - 2 >= 0 && currRow - 1 >= 0) && table[currRow-1][currCol-2] == 0){
        arr[2] = 1;
    }
    else{
        arr[2] = 0;
    }
   
    //2 left 1 down
    if((currCol - 2 >= 0 && currRow + 1 < m) && table[currRow+1][currCol-2] == 0){
        arr[3] = 1;
    }
    else{
        arr[3] = 0;
    }
    //2 up 1 right    
    if((currCol + 1 < n && currRow - 2 >= 0) && table[currRow-2][currCol+1] == 0){
        arr[4] = 1;
    }
    else{
        arr[4] = 0;
    }
   
    //2 up 1 left
    if((currCol - 1 >= 0 && currRow - 2 >= 0) && table[currRow-2][currCol-1] == 0){
        arr[5] = 1;
    }
    else{
        arr[5] = 0;
    }
   
    //2 down 1 right
    if((currCol + 1 < n && currRow + 2 < m) && table[currRow+2][currCol+1] == 0){
        arr[6] = 1;
    }
    else{
        arr[6] = 0;
    }
   
    //2 down 1 left
    if((currCol - 1 >= 0 && currRow + 2 < m) && table[currRow+2][currCol-1] == 0){
        arr[7] = 1;
    }
    else{
        arr[7] = 0;
    }
 
    return arr;
}
 
int *moveKnight(int val, int row, int col, int *pair){
   
    if(val == 0){
        pair[0] = row - 1;
        pair[1] = col + 2;
        return pair;
    }
    if(val == 1){
        pair[0] = row + 1;
        pair[1] = col + 2;
        return pair;
    }
    if(val == 2){
        pair[0] = row - 1;
        pair[1] = col - 2;
        return pair;
    }
    if(val == 3){
        pair[0] = row + 1;
        pair[1] = col - 2;
        return pair;
    }
    if(val == 4){
        pair[0] = row - 2;
        pair[1] = col + 1;
        return pair;
    }
    if(val == 5){
        pair[0] = row - 2;
        pair[1] = col - 1;
        return pair;
    }
    if(val == 6){
        pair[0] = row + 2;
        pair[1] = col + 1;
        return pair;
    }
    if(val == 7){
        pair[0] = row + 2;
        pair[1] = col - 1;
        return pair;
    }
    pair[0] = row;
    pair[1] = col;
    return pair;
}
 
void printTable(int **table, int m, int n){
    int i;
    int j;
    for(i = 0; i < m; i++){
        printf("PID %i:   ", getpid());
        //fflush(stdout);
        for(j = 0; j < n; j++){
            if(table[i][j] == 0){
                printf(".");
            }
            else{
                printf("k");
            }
        }
        printf("\n");
        fflush(stdout);
    }
    fflush(stdout);
}
 
void move(int **table, int m, int n, int *pair, int *arr, int display){
    //Check if each possible movement direction exists then make a fork based on this
    //And if the posssibles spaces haven't been seen yet
    int possibleSpaces = 0;
    int deadEnd = 0;
    int currRow = 0;
    int currCol = 0;
    int i;
    int j;
    printf("PID %i: Solving the knight's tour problem for a %ix%i board\n",getpid(), m,n);
    fflush(stdout);
    //Create an (m*n) * 2 table of all fd pipes
    int totalSize = m*n;
    int **fds = (int **)calloc(totalSize, sizeof(int*));
    for(i = 0; i < totalSize; i++){
        fds[i] = (int *)calloc(2, sizeof(int));
    }
    
    int numChildren = 0;
    int maxSeen = 0;
    int order = 1;
    int moves = 0;
    
    int parentFds[] = {-1,-1};
   
    while(!deadEnd){
        //fflush(stdout);
        table[currRow][currCol] = order;
        moves += 1;
        order += 1;
        int *spaces = countMoves(table, currRow, currCol, m, n, arr);
        
        possibleSpaces = sum(spaces,8);
        
        if(possibleSpaces > 1){
            printf("PID %i: %i moves possible after move #%i\n", getpid(),possibleSpaces,moves);
            //fflush(stdout);
            if(display){
                printTable(table,m,n);
            }
            fflush(stdout);
        }

        if(possibleSpaces > 1){ //make a fork for each of the possible routes
            for(i = 0; i < 8; i++){
                if(spaces[i] == 1){
                    if(pipe(fds[numChildren]) == -1){
                        fprintf(stderr, "pipe failed\n");
                        //return EXIT_FAILURE;
                    }
                    
                    fflush(stdout);
                    pid_t pid = fork();
                    if(pid < 0){
                        fprintf(stderr, "fork failed\n");
                        //return EXIT_FAILURE;
                    }
                    else if(pid > 0){//parent process
                        //if two children uses 0/1
                        numChildren += 1;
                    }
                    else{ //child process
                        parentFds[0] = fds[numChildren][0];
                        parentFds[1] = fds[numChildren][1];

                        numChildren = 0;
                        int *pos = moveKnight(i, currRow, currCol, pair);
                        currRow = pos[0];
                        currCol = pos[1];
                        break;
                    }
                }
            }
            
            //parent reading from each child
            //store largest value among children
            int amountSeenByChildren[numChildren+1];
            int largest = 0;
            if(numChildren > 0){
                for(i = 0; i < numChildren; i++){
                    wait(NULL);
                    //fflush(stdout);
                }
                
                for(i = 0; i < numChildren; i++){
                    amountSeenByChildren[i] = 0;
                    
                    close(fds[i][1]); //close write end
                    read(fds[i][0], &amountSeenByChildren[i], sizeof(int)); //read from child
                    close(fds[i][0]); //close read end
                    //fflush(stdout);
                    printf("PID %i: Received %i from child\n", getpid(),amountSeenByChildren[i]);
                    //fflush(stdout);
                    if(amountSeenByChildren[i] > largest){
                        largest = amountSeenByChildren[i];
                    }
                }
            
            
                
                //if an intermediate node write the max value
                if((parentFds[0]>0) && (parentFds[1]>0)){
                    //fflush(stdout);
                    printf("PID %i: All child processes terminated; sent %i on pipe to parent\n", getpid(), largest);
                    fflush(stdout);
                    close(parentFds[0]); //close read end
                    write(parentFds[1], &largest, sizeof(int)); //write value to parent
                    close(parentFds[1]); //close write end
                }
                
                if(largest > maxSeen){
                    maxSeen = largest;
                }
                deadEnd = 1;
            }
            
            
        }
        else if(possibleSpaces == 1){ //If only one space follow that path
            for(i = 0; i < 8; i++){
                if(spaces[i] == 1){
                    int *pos = moveKnight(i, currRow, currCol, pair);
                    currRow = pos[0];
                    currCol = pos[1];
                    break;
                }
            }
           
        }
        else{ //deadend reached, child wants to write to parent
            deadEnd = 1;
            
            //fflush(stdout);
            printf("PID %i: Dead end after move #%i\n", getpid(), moves);
            fflush(stdout);
            if(display){
                printTable(table,m,n);
            }
            //fflush(stdout);
            int seen = 0;
            for(i = 0; i < m; i++){
                for(j = 0; j < n; j++){
                    if(table[i][j] != 0){
                        seen += 1;
                    }
                }
            }
            //write info to parents 
            if(parentFds[0] && parentFds[1]){
                //fflush(stdout);
                printf("PID %i: Sent %i on pipe to parent\n", getpid(),seen);
                fflush(stdout);
                close(parentFds[0]); //close read end
                write(parentFds[1], &seen, sizeof(int));
                close(parentFds[1]); //close write end
            }
        }
        //fflush(stdout);
    }
   
    for(i = 0; i < totalSize; i++){
        free(fds[i]);
    }
   
    free(fds);
   
    if((parentFds[0]<0) && (parentFds[1]<0)){
            printf("PID %i: Best solution found visits %i squares (out of %i)\n",getpid(),maxSeen,totalSize);
            fflush(stdout);
    }
    else{
        kill(getpid(), SIGTERM);
    }
    
    
}

void moveNP(int **table, int m, int n, int *pair, int *arr, int display){
    //Check if each possible movement direction exists then make a fork based on this
    //And if the posssibles spaces haven't been seen yet
    int possibleSpaces = 0;
    int deadEnd = 0;
    int currRow = 0;
    int currCol = 0;
    int i;
    int j;
    printf("PID %i: Solving the knight's tour problem for a %ix%i board\n",getpid(), m,n);
    fflush(stdout);
    //Create an (m*n) * 2 table of all fd pipes
    int totalSize = m*n;
    int **fds = (int **)calloc(totalSize, sizeof(int*));
    for(i = 0; i < totalSize; i++){
        fds[i] = (int *)calloc(2, sizeof(int));
    }
    
    int numChildren = 0;
    int maxSeen = 0;
    int order = 1;
    int moves = 0;
    
    int parentFds[] = {-1,-1};
   
    while(!deadEnd){
        table[currRow][currCol] = order;
        moves += 1;
        order += 1;
        int *spaces = countMoves(table, currRow, currCol, m, n, arr);
        
        possibleSpaces = sum(spaces,8);
        
        if(possibleSpaces > 1){
            printf("PID %i: %i moves possible after move #%i\n", getpid(),possibleSpaces,moves);
            fflush(stdout);
            if(display){
                printTable(table,m,n);
            }
        }

        if(possibleSpaces > 1){ //make a fork for each of the possible routes
            for(i = 0; i < 8; i++){
                if(spaces[i] == 1){
                    if(pipe(fds[numChildren]) == -1){
                        fprintf(stderr, "pipe failed\n");
                        fflush(stdout);
                        //return EXIT_FAILURE;
                    }
                    
                    fflush(stdout);
                    pid_t pid = fork();
                    if(pid < 0){
                        fprintf(stderr, "fork failed\n");
                        fflush(stdout);
                        //return EXIT_FAILURE;
                    }
                    else if(pid > 0){//parent process
                       
                        //if two children uses 0/1
                        wait(NULL);
                        numChildren += 1;
                    }
                    else{ //child process
                        parentFds[0] = fds[numChildren][0];
                        parentFds[1] = fds[numChildren][1];

                        numChildren = 0;
                        int *pos = moveKnight(i, currRow, currCol, pair);
                        currRow = pos[0];
                        currCol = pos[1];
                        break;
                    }
                }
            }
            
            //parent reading from each child
            //store largest value among children
            int amountSeenByChildren[numChildren+1];
            int largest = 0;
            if(numChildren > 0){
                
                for(i = 0; i < numChildren; i++){
                    amountSeenByChildren[i] = 0;
                    
                    close(fds[i][1]); //close write end
                    read(fds[i][0], &amountSeenByChildren[i], sizeof(int)); //read from child
                    close(fds[i][0]); //close read end
                    printf("PID %i: Received %i from child\n", getpid(),amountSeenByChildren[i]);
                    fflush(stdout);
                    if(amountSeenByChildren[i] > largest){
                        largest = amountSeenByChildren[i];
                    }
                }
            
            
                
                //if an intermediate node write the max value
                if((parentFds[0]>0) && (parentFds[1]>0)){
                    printf("PID %i: All child processes terminated; sent %i on pipe to parent\n", getpid(), largest);
                    fflush(stdout);
                    close(parentFds[0]); //close read end
                    write(parentFds[1], &largest, sizeof(int)); //write value to parent
                    close(parentFds[1]); //close write end
                }
                
                if(largest > maxSeen){
                    maxSeen = largest;
                }
                deadEnd = 1;
            }
            
            
        }
        else if(possibleSpaces == 1){ //If only one space follow that path
            for(i = 0; i < 8; i++){
                if(spaces[i] == 1){
                    int *pos = moveKnight(i, currRow, currCol, pair);
                    currRow = pos[0];
                    currCol = pos[1];
                    break;
                }
            }
           
        }
        else{ //deadend reached, child wants to write to parent
            deadEnd = 1;
            
            
            printf("PID %i: Dead end after move #%i\n", getpid(), moves);
            fflush(stdout);
            
            if(display){
                printTable(table,m,n);
            }
            int seen = 0;
            for(i = 0; i < m; i++){
                for(j = 0; j < n; j++){
                    if(table[i][j] != 0){
                        seen += 1;
                    }
                }
            }
            //write info to parents 
            if(parentFds[0] && parentFds[1]){
                printf("PID %i: Sent %i on pipe to parent\n", getpid(),seen);
                fflush(stdout);
                close(parentFds[0]); //close read end
                write(parentFds[1], &seen, sizeof(int));
                close(parentFds[1]); //close write end
                
            }
        }
    }
   
    for(i = 0; i < totalSize; i++){
        free(fds[i]);
    }
   
    free(fds);
   
    if((parentFds[0]<0) && (parentFds[1]<0)){
            printf("PID %i: Best solution found visits %i squares (out of %i)\n",getpid(),maxSeen,totalSize);
            fflush(stdout);
    }
    else{
        kill(getpid(), SIGTERM);
    }
    
    
}
 
int main(int argc, char *argv[])
{
    //int display = 0;
    
    if(argc != 3){
        fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: a.out <m> <n>\n");
        fflush(stdout);
	    return EXIT_FAILURE;
    }
    
    
    int m = atoi(argv[1]);
    int n = atoi(argv[2]);
    int i;
    int j;
   
    if(m < 3 || n < 3){
        fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: a.out <m> <n>\n");
        fflush(stdout);
	    return EXIT_FAILURE;
    }
   
    int display = 0;
    #ifdef DISPLAY_BOARD
        display = 1;
    #endif
   
    //Create an MxN board
    //Each space on the board stores if the space has been visited or not
    //0 = not visited, 1 = visited
    int **table = calloc(m, sizeof(int *));
    for(i = 0; i <m; i++){
        table[i] = calloc(n, sizeof(int));
    }
   
    //Set the initial values for each space on the board
    for(i = 0; i < m; i++){
        for(j = 0; j < n; j++){
            table[i][j] = 0;
        }
    }
   
    int *pair = calloc(2, sizeof(int *));
    int *arr = calloc(8, sizeof(int *));
   
   
    #ifdef NO_PARALLEL
        moveNP(table, m, n, pair, arr, display);
    #else
        move(table, m, n, pair, arr, display);
    #endif

   
    //Free all dynamically allocated variables
    for(i = 0; i < m; i++){
       free(table[i]);  
    }
 
    free(table);
    free(pair);
    free(arr);
   
    return 0;
}