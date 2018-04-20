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
#include <pthread.h>

int max_squares = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

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

void printDeadBoards(int ***dead_boards, int total, int m, int n, int wanted){
    int i;
    int j;
    int k;
    int seen;
    for(k = 0; k < total; k++){
        seen = 0;
        for(i = 0; i <m; i++){
            for(j = 0; j < n; j++){
                if(dead_boards[k][i][j] > 0){
                    seen += 1;
                }
            }
        }
        
        if(seen >= wanted){
            for(i = 0; i < m; i++){
                if(i == 0)
                    printf("TID %lu: > ", pthread_self());
                else
                    printf("TID %lu:   ", pthread_self());
                for(j = 0; j < n; j++){
                    if(dead_boards[k][i][j] == 0){
                        printf(".");
                    }
                    else{
                        printf("k");
                        //printf("%i", dead_boards[k][i][j]);
                    }
                }
                printf("\n");
                fflush(stdout);
            }
            //printf("\n");
            fflush(stdout);
        }
    }
    fflush(stdout);
}
 
void printTable(int **table, int m, int n){
    int i;
    int j;
    for(i = 0; i < m; i++){
        printf("TID %lu:   ", pthread_self());
        for(j = 0; j < n; j++){
            if(table[i][j] == 0){
                printf(".");
            }
            else{
                printf("k");
                //printf("%i", table[i][j]);
            }
        }
        printf("\n");
        fflush(stdout);
    }
    fflush(stdout);
}

struct arg_struct {
    int **table;
    int m;
    int n;
    //int *pair;
    //int *arr;
    int display;
    int currRow;
    int currCol;
    int order;
    int moves;
    int ***dead_boards;
    int *index;
    int wanted;
};

//void startMove(int **table, int m, int n, int *pair, int *arr, int display, currRow, currCol, order){
void* startMove(void *arguments){
    //printf("----\n");
    struct arg_struct *currArgs = arguments;
    pthread_t thread1, thread2;
    
    int deadEnd = 0;
    int possibleSpaces = 0;
    int firstSeen = 0;
    int i = 0;
    int j = 0;
    int k = 0;
    int *pair = calloc(2, sizeof(int *));
    int *arr = calloc(8, sizeof(int *));
    
    while(!deadEnd){
        currArgs->table[currArgs->currRow][currArgs->currCol] = currArgs->order;
        currArgs->order += 1;
        currArgs->moves += 1;
        int *spaces = countMoves(currArgs->table, currArgs->currRow, currArgs->currCol, currArgs->m, currArgs->n, arr);
        possibleSpaces = sum(spaces,8);
        
        if(possibleSpaces > 1){
            printf("TID %lu: %i moves possible after move #%i; creating threads\n", pthread_self(),possibleSpaces,currArgs->moves);
            fflush(stdout);
            if(currArgs->display){
                printTable(currArgs->table,currArgs->m,currArgs->n);
            }
            fflush(stdout);
        }

        if(possibleSpaces > 1){ //make a thread for each of the possible routes
            for(i = 0; i < 8; i++){
                if(spaces[i] == 1){
                    //Create a deep copy of the initial table
                    int **table2 = calloc(currArgs->m, sizeof(int *));
                    for(j = 0; j < currArgs->m; j++){
                        table2[j] = calloc(currArgs->n, sizeof(int));
                    }
                    //Set the initial values for each space on the board
                    for(j = 0; j < currArgs->m; j++){
                        for(k = 0; k < currArgs->n; k++){
                            table2[j][k] = currArgs->table[j][k];
                        }
                    }

                    int *pos = moveKnight(i, currArgs->currRow, currArgs->currCol, pair);
                    //printf("TID %lu: Can move to: (%i,%i)\n", pthread_self(), pos[0], pos[1]);
                    
                    struct arg_struct *newArgs = malloc(sizeof(struct arg_struct));
                    newArgs->table = table2;
                    newArgs->m = currArgs -> m;
                    newArgs->n = currArgs -> n;
                    newArgs->display = currArgs -> display;
                    newArgs->order = currArgs -> order;
                    newArgs->moves = currArgs -> moves;
                    newArgs->currRow = pos[0];
                    newArgs->currCol = pos[1];
                    newArgs->dead_boards = currArgs->dead_boards;
                    newArgs->index = currArgs->index;
                    newArgs->wanted = currArgs->wanted;
                    
                    //Assumes that only two max children will exist
                    if(!firstSeen){
                        fflush(stdout);
                        pthread_create(&thread1, NULL, startMove, (void *)newArgs);
                        firstSeen += 1;
                    }
                    else{
                        fflush(stdout);
                        pthread_create(&thread2, NULL, startMove, (void *)newArgs);
                    }
                }
            }
            
            //Let the children finish running
            pthread_join(thread1, NULL);
            pthread_join(thread2, NULL);
            deadEnd = 1;
            
            
        }
        else if(possibleSpaces == 1){ //If only one space follow that path
            for(i = 0; i < 8; i++){
                if(spaces[i] == 1){
                    int *pos = moveKnight(i, currArgs->currRow, currArgs->currCol, pair);
                    currArgs->currRow = pos[0];
                    currArgs->currCol = pos[1];
                    break;
                }
            }
           
        }
        else{ //deadend reached, only at this point do we update max_squares / dead board list
            deadEnd = 1;
            printf("TID %lu: Dead end after move #%i\n", pthread_self(), currArgs->moves);
            fflush(stdout);
            if(currArgs->display){
                printTable(currArgs->table,currArgs->m,currArgs->n);
            }
            int seen = 0;
            for(i = 0; i < currArgs->m; i++){
                for(j = 0; j < currArgs->n; j++){
                    if(currArgs->table[i][j] != 0){
                        seen += 1;
                    }
                }
            }
            //printf("TID %lu: seen %i squares \n", pthread_self(), seen);
            
            //CRITICAL SECTION
            pthread_mutex_lock(&lock);
            //Update global max_squares
            if(seen > max_squares){
                max_squares = seen;
            }
            
            //Add board to list of deadboards
            if(seen >= currArgs->wanted){
                for(i = 0; i < currArgs->m; i++){
                    for(j = 0; j < currArgs->n; j++){
                        currArgs->dead_boards[*(currArgs->index)][i][j] = currArgs->table[i][j];
                    }
                }
            }
            
            *(currArgs->index) += 1;
            pthread_mutex_unlock(&lock);
     
        }
    }
    //Free all dynamically allocated variables
    for(j = 0; j < currArgs->m; j++){
       free(currArgs->table[j]);  
    }
    free(currArgs->table);
    free(currArgs);
    free(pair);
    free(arr);
    
    return NULL;
}
 
int main(int argc, char *argv[])
{
    //int display = 0;
    
    if(argc < 3 || argc > 4){
        fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: a.out <m> <n> [<k>]\n");
        fflush(stdout);
	    return EXIT_FAILURE;
    }
    
    
    
    int m = atoi(argv[1]);
    int n = atoi(argv[2]);
    int wanted = 1;
    if(argc == 4){
        wanted = atoi(argv[3]);
        if(wanted > m * n || wanted < 1){
            fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: a.out <m> <n> [<k>]\n");
            fflush(stdout);
    	    return EXIT_FAILURE;
        }
    }
    
    
    
    int i;
    int j;
    int k;
   
    if(m < 3 || n < 3){
        fprintf(stderr, "ERROR: Invalid argument(s)\nUSAGE: a.out <m> <n> [<k>]\n");
        fflush(stdout);
	    return EXIT_FAILURE;
    }
   
    int display = 0;
   
    //Create an MxN board
    //Each space on the board stores if the space has been visited or not
    //0 = not visited, 1 = visited
    int **table = calloc(m, sizeof(int *));
    for(i = 0; i < m; i++){
        table[i] = calloc(n, sizeof(int));
    }
    
    //Set the initial values for each space on the board
    for(i = 0; i < m; i++){
        for(j = 0; j < n; j++){
            table[i][j] = 0;
        }
    }
    
    int total = 200;
    
    
    int ***dead_boards = calloc(total, sizeof(int **));
    for(i = 0; i < total; i++){
        dead_boards[i] = calloc(m, sizeof(int *));
        for(j = 0; j < m; j++){
            dead_boards[i][j] = calloc(n, sizeof(int));
        }
    }
    
    for(i = 0; i < total; i++){
        for(j = 0; j < m; j++){
            for(k = 0; k < n; k++){
                dead_boards[i][j][k] = 0;
            }
        }
    }
   
    printf("TID %lu: Solving the knight's tour problem for a %ix%i board\n",pthread_self(), m,n);
    int index = 0;
    
    struct arg_struct *newArgs = malloc(sizeof(struct arg_struct));
    newArgs->table = table;
    newArgs->m = m;
    newArgs->n = n;
    newArgs->display = display;
    newArgs->currRow = 0;
    newArgs->currCol = 0;
    newArgs->order = 1;
    newArgs->moves = 0;
    newArgs->dead_boards = dead_boards;
    newArgs->index = &index;
    newArgs->wanted = wanted;
    
    
    
    startMove((void *)newArgs);
    printf("TID %lu: Best solution found visits %i squares (out of %i)\n",pthread_self(),max_squares,m*n);
    printDeadBoards(dead_boards, total, m, n, wanted);
    //Free all dynamically allocated variables
    for(i = 0; i < total; i++){
        for(j = 0; j < m; j++){
           free(dead_boards[i][j]);  
        }
        free(dead_boards[i]);
    }
    free(dead_boards);
    
    return 0;
}