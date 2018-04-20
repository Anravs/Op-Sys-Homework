#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
int main(int argc, char *argv[])
{
    if(argc < 2 || argc > 3){
        fprintf(stderr, "ERROR: Number of argument inputs must be either 2 or 3\n");
	    return EXIT_FAILURE;
    }
    
    
   int i;
   FILE *file;
   
   
   //file = fopen(argv[1], "r");
   
   /*if(file == NULL && errno == ENOENT){
        fprintf(stderr, "File does not exist\n");
        fflush(stdout);
	    return EXIT_FAILURE;
   }*/
   
   int limit = 32;
   
   char **words = (char **)calloc(limit, sizeof(char*));
   int *counts = (int *)calloc(limit, sizeof(int));
   printf("Allocated initial parallel arrays of size %i.\n", limit);
   fflush(stdout);
   
   int freeSpace = 0;
   int numWords = 0;
   int uniqueWords = 0;
   
   
   int wordIndex = 0;
   
   char currWord[80]; //each word is max length 79 + "\0" character
   
   char ch;
   
   char* inputDir = argv[1];
   
   DIR * dir = opendir(inputDir);
   
   if(dir == NULL){
       fprintf(stderr, "ERROR: Directory does not exist\n");
	   return EXIT_FAILURE;
   }
   
   struct dirent * currFile;
   while((currFile = readdir(dir)) != NULL){
       struct stat buffer;
       chdir(inputDir);
       int rc = lstat(currFile->d_name, &buffer);
       if(rc == -1){
           fprintf(stderr, "ERROR: lstat didn't work\n");
	       return EXIT_FAILURE;
       }
       
       if(S_ISREG(buffer.st_mode)){
           file = fopen(currFile->d_name, "r");
           
           if(file == NULL){
               fprintf(stderr, "ERROR: file open failed\n");
	           return EXIT_FAILURE;
           }
           
           while(!feof(file)){
               ch = getc(file);
               if(isalnum(ch)){ //Add this to the counter of current words
                    currWord[wordIndex] = ch;
                    wordIndex += 1;
               }
               else{ //not a word so pass in current word and reset char array
                   //char* token = NULL;
                   char newWord[wordIndex+1];
                   for(i = 0; i < wordIndex; i++){
                       newWord[i] = currWord[i];
                   }
                   
                   
                   strcpy(currWord,"");
                   
                   newWord[wordIndex] = '\0';
                   
                   wordIndex = 0;
                   
                   if(strlen(newWord) > 1){ //If there were two puctuations following each other, don't want to throw in a word of a length 0
                   
                       char* token = newWord;
                       
                       if(strlen(token) <= 80){
                           int found = 0;
                           
                           for( i = 0; i < freeSpace; i = i +1){ //linear search to see if the word has already been seen
                                if(freeSpace > 0 && !strcmp(token, words[i])){
                                    counts[i] = counts[i] + 1;
                                    found = 1;
                                }
                            }
                            
                            if(!found){ //if the word is a new word add it to the arrays
                                if(freeSpace > 0 && (freeSpace % limit) == 0){ //reallocate more space if necessary
                                    limit += 32;
                                    words = (char**) realloc(words, limit * sizeof(char*));
                                    counts = (int*) realloc(counts, limit * sizeof(int));
                                    printf("Re-allocated parallel arrays to be size %i.\n", limit);
                                    fflush(stdout);
                                }
                                
                                //save the word dynamically based on side of the word
                                words[freeSpace] = (char*) malloc((strlen(token)+1) * sizeof(char));
                                strcpy(words[freeSpace],token);
                                counts[freeSpace] = 1; //counts[freeSpace] + 1;
                                freeSpace += 1;
                                uniqueWords += 1;
                            }
                            
                            numWords += 1;
                       }
                   }
                }
            }
       }
   }
   
   printf("All done (successfully read %i words; %i unique words).\n", numWords, uniqueWords);
   fflush(stdout);
   
   if(argc == 2){
       printf("All words (and corresponding counts) are:\n");
       for( i = 0; i < freeSpace; i = i+1){
            printf("%s -- %i\n",words[i],counts[i]);
            fflush(stdout);
       }
   }
   else{
       int maxShow = atoi(argv[2]);
       char *subArray[maxShow+1];
       memcpy(subArray, words, maxShow);
       printf("First %i words (and corresponding counts) are:\n", maxShow);
       for(i = 0; i < freeSpace; i = i+1){
           if(i < maxShow){
               printf("%s -- %i\n",words[i],counts[i]);
               fflush(stdout);
           }
       }
       printf("Last %i words (and corresponding counts) are:\n", maxShow);
       for(i = 0; i < freeSpace; i = i+1){
           if(i >= (freeSpace - maxShow)){
               printf("%s -- %i\n",words[i],counts[i]);
               fflush(stdout);
           }
       }
   }

   for(i = 0; i < freeSpace; i++){
       free(words[i]);
   }

   free(words);
   free(counts);
   
   fclose(file);
   closedir(dir);
   
   return 0;
}