#define MAX_BUF 257
#define WORD_SIZE 64

#include "cachelab.h"

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

unsigned setSize=0, numLine=0, blockSize=0;
unsigned numSize, numBlock;
char verbose = 0;
FILE * traceFileFP=NULL;

char buf[MAX_BUF];
unsigned numHit=0, numMiss=0, numEviction=0;

// cache[i][j][63:63-tagSize]   => tag
// cache[i][j][0]               => valid bit
size_t ** cache;
unsigned ** counter;  // for LRU eviction policy
size_t tagMask; // extract tag from address
size_t setMask; // extract set from address
// omit block bit for Part A
// size_t blockMask;


void printHelpMessage(){
    printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n");
    printf("Options:\n");
    printf("  -h\t\tPrint this help message.\n");
    printf("  -v\t\tOptional verbose flag.\n");
    printf("  -s <num>\tNumber of set index bits.\n");
    printf("  -E <num>\tNumber of lines per set.\n");
    printf("  -b <num>\tNumber of block offset bits.\n");
    printf("  -t <file>\tTrace file.\n\n");

    printf("Examples:\n");
    printf("  linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n");
    printf("  linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

void checkArgs(char * traceFile){
    if(setSize==0 || numLine==0 || blockSize==0 || traceFile==NULL){
        printf("./csm: Missing required command line argument\n");
        printHelpMessage();
        exit(0);
    }

    traceFileFP = fopen(traceFile, "r");
    if(!traceFileFP){
        printf("%s: No such file or directory\n", traceFile);
        exit(0);
    }
}

void parseArgs(int argc, char * argv[]){
    enum argType {
        NONE, NSET, NLine, NBLOCK, TFILE
    };

    char * traceFile=NULL;
    int i=1;
    enum argType argType=NONE;

    while(i<argc){
        if(!strcmp(argv[i], "-s")){
            argType = NSET;
        } else if(!strcmp(argv[i], "-E")){
            argType = NLine;
        } else if(!strcmp(argv[i], "-b")){
            argType = NBLOCK;
        } else if(!strcmp(argv[i], "-t")){
            argType = TFILE;
        } else if(!strcmp(argv[i], "-v")){
            verbose = 1;
        } else if (!strcmp(argv[i], "-h")) {
            printHelpMessage();
            exit(0);
        } else {
            switch (argType)
            {
            case NONE:
                printf("./csim: invalid option -- %s\n", argv[i]);
                printHelpMessage();
                exit(0);
            case NSET:
                sscanf(argv[i], "%u", &setSize);
                argType = NONE;
                break;

            case NLine:
                sscanf(argv[i], "%u", &numLine);
                argType = NONE;
                break;

            case NBLOCK:
                sscanf(argv[i], "%u", &blockSize);
                argType = NONE;
                break;
            
            case TFILE:
                traceFile = argv[i];
                argType = NONE;
                break;
            }
        }
        i++;
    }
    checkArgs(traceFile);
}

void initCache(){
    unsigned i, j;

    numSize = 1<<setSize;
    numBlock = 1<<blockSize;

    cache = malloc(sizeof(void *) * (numSize));
    counter = malloc(sizeof(void *) * (numSize));
    for(i=0;i<numSize; i++){
        cache[i] = malloc(sizeof(size_t) * numLine);
        counter[i] = malloc(sizeof(unsigned) * numLine);
        for(j=0; j<numLine; j++){
            cache[i][j] = 0;
            counter[i][j] = 0;
        }
    }

    tagMask = ~((1<<(setSize + blockSize)) - 1);
    setMask = ~(((1<<blockSize) - 1) | tagMask);
}

void extractSetAndTag(size_t address, size_t * setPtr, size_t * tagPtr){
    *setPtr = (address & setMask)>>blockSize;
    *tagPtr = address & tagMask;
}

unsigned cacheLoad(size_t set, size_t tag){
    size_t * line = cache[set], block;
    unsigned i, replaceBlock = numLine;
    
    for(i=0; i<numLine; i++){
        block = line[i];
        if((block & 0x1) && !((block&tagMask)^tag)){
            // hit
            strcat(buf, " hit");
            numHit++;
            return i;
        }

        if(replaceBlock == numLine && !(block & 0x1))
            replaceBlock = i;
    }

    strcat(buf, " miss");
    numMiss++;

    if(replaceBlock == numLine) {
        // evict
        strcat(buf, " eviction");
        numEviction++;

        unsigned * setCounter = counter[set];
        unsigned cnt = 0, cur_cnt;
        for(i=0; i<numLine; i++){
            block = line[i];
            cur_cnt = setCounter[i];
            if(cur_cnt > cnt){
                replaceBlock = i;
                cnt = cur_cnt;
            }
        }
    }

    // allocate
    line[replaceBlock] = tag | 0x1;

    return replaceBlock;
}

unsigned cacheModify(size_t set, size_t tag){
    unsigned replaceBlock = cacheLoad(set, tag);

    strcat(buf, " hit");
    numHit++;
    
    return replaceBlock;
}

void updateCounter(unsigned blockId, size_t set){
    unsigned * setCounter = counter[set], i;
    size_t * line = cache[set];

    for(i=0; i<numLine; i++){
        if(i == blockId){
            setCounter[i] = 1;
        } else if((line[i]&0x1)){
            setCounter[i]++; 
        }
    }
}

int main(int argc, char * argv[])
{
    if(sizeof(size_t)<<3!=WORD_SIZE){
        printf("This program supposes that the size of size_t is 64 bits.\nWhile this host doesn't hold this assumption.\n");
        printf("sizeof(size_t)=%ld\n", sizeof(size_t));
        exit(0);
    }
    char operation;
    size_t address;
    unsigned size;

    size_t tag, set;
    unsigned replaceBlock;

    parseArgs(argc, argv);
    // printf("set size: %u, num line: %u, block size: %u\n", setSize, numLine, blockSize);
    initCache();

    while (fgets(buf, MAX_BUF-1, traceFileFP)){
        if(buf[0] == 'I' || sscanf(buf, " %c %lx,%u\n", &operation, &address, &size)!=3)
            continue;

        extractSetAndTag(address, &set, &tag);
        // printf("address: %lx ==> extracted set: %lx, tag: %lx\n", address, set, tag);
        sprintf(buf, "%c %lx,%u", operation, address, size);
        switch (operation)
        {
        case 'L':
        case 'S':
            replaceBlock = cacheLoad(set, tag);
            break;
        
        case 'M':
            replaceBlock = cacheModify(set, tag);
            break;

        default:
            printf("unsupport operation: %c", operation);
            exit(0);
        }
        updateCounter(replaceBlock, set);
        if(verbose) printf("%s\n", buf);
    }
    printSummary(numHit, numMiss, numEviction);
    fclose(traceFileFP);
    return 0;
}
