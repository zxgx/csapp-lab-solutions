# CSAPP lab

My solutions for the labs of [CSAPP 3rd edition](http://csapp.cs.cmu.edu/3e/labs.html).

Just for fun!

Environment: 
- Linux 5.10.16.3-microsoft-standard-WSL2
- Ubuntu 20.04 LTS

Below are the notes for each lab dir.

## lab 1: data lab
Just finish each function in `bits.c`. 
Google maybe helpful when getting in trouble. :)

中文参考：https://zhuanlan.zhihu.com/p/59534845

## lab 2: bomb lab
1. Use `objdump` to generate x86_64 asm code. 
```bash
cd lab/bomb
objdump -d ./bomb > bomb.asm
```

2. Read the `<phase_x>` (x = 1 - 6) segments in the `bomb.asm` file. Converting the asm code into c code might be helpful.

The ciphers are in the `cipher` file. Several phases have multiple solutions.

## lab 3: attack lab
1. Use `objdump` to generate x86_64 asm code.
```bash
cd lab/target1
objdump -d ./ctarget > ctarget.asm # Code Injection Attacks
objdump -d ./rtarget > rtarget.asm # Return-oriented Programming
```

2. Utilize the buffer overflow trick  in the `getbuf` segment to insert desirable codes. 

The instructions are explained in the file `attacklab.pdf`.
My solutions are in `c1 - c4`. I did not work on phase 5 since it only has 5 scores and is similar to phase 3. 

I could not resolve the second attack. The generated disassembly code is in `c2.d` and has binary bits different from the correct answers in `c2`. I suppose this is due to the gcc or objdump version.

中文参考：https://zhuanlan.zhihu.com/p/560012698

## lab 4: arch lab
Instructions for this lab are described in `archlab-handout/archlab.pdf`.
I don't want to install Tcl/Tk, so I just comment out the installation options for GUI in `sim/Makefile`, `sim/seq/Makefile` and `sim/pipe/Makefile`. For better debugging, one may need to enable GUI mode.

### Part A: sim/misc
My solutions are in `sum.ys`, `rsum.ys` and `copy.ys`.
The basic Y86-64 program structure is illustrated in Section 4.1.5 of the textbook.

It seems that there isn't any way to autograde this part without a CMU student id.
Therefore, to evaluate the Y86-64 codes, I could only compare the memory and registers contents after the program halts.

### Part B: sim/seq
My solution for iaddq is appended in `seq-full.hcl`.

Altering several lines of each stage could fulfill this part.

Tips for evaluation are described in `archlab.pdf`.

### Part C: sim/pipe
The benchmark of my solutions. All the files are listed in the `sim/pipe` dir.
- iaddq.ys: add iaddq instruction for pipelined Y86-64 and use it in ncopy.
- cmov.ys: translate the branch statement into cmov instruction.
- data-hazard.ys: separate the read/write instructions to eliminate the data hazard (more specifically, load/use hazard).
- unrolling-2.ys: unroll the loop by 2 elements.
- unrolling-2-v2.ys: remove the cmov instruction.
- unrolling-4.ys: unroll the loop by 4 elements.

|  setup      | benchmark avg CPE |
| :-------:   | :---------------: |
|  original   |    15.18          |
|  iaddq      |    12.70          |
|  cmov       |    13.26          |
| data-hazard |    12.26          |
| unrolling-2 |    10.75          |
| unrolling-2-v2 | 10.25          |
| unrolling-4 |     9.27          |
| unrolling-8 |     8.81          |

This lab and the corresponding Chapter 4 and Chapter 5 are miserable and kind of confusing.
I suppose there are a large number of topics which are not discussed, and the discussed issues in the two chapters also contains unmentioned details.


## lab 5: cache lab
The setup and evaluation instructions are illustrated in the file `cachelab.pdf`.

### Part A: csim.c
The cache simulator is easy. Load operation on cache has a pattern similar to Save operation, while the Modify operation consists of a Load operation followed by a Save operation.

### Part B: trans.c
#### 32x32 matrix  
The solution depends on splitting the matrix into blocks.  
To resolve the conflicts on the diagonal, we can resort to the loop unrolling technique.

#### 64x64 matrix
The memory address pattern of this matrix is tricky, as each 4 lines share the same cache address, which makes the unrolling technique in the previous one invalid.
To resolve this problem, we need some subtle operations inside each blocks, that is, copy/transpose each block by 4x4 sub-blocks.
By separately handling the diagonal blocks, my solution further reduces 40 cache misses. 

My digest: try to analyse the address pattern of the first two blocks. The key problem is how to eliminate the cache miss for the bottom right sub-block.

#### 61x67 matrix
Since there is no particular pattern on the memory address, we could resolve this question by implementing a general block-based matrix copy and conducting a series of experiments to gain the result.

中文参考：https://zhuanlan.zhihu.com/p/456858668


## lab 6: shell lab
Instructions for setup and evaluation are listed in `shlab-handout/shlab.pdf`.

Generally, there are two differences between my implementation and the requirements.
1. I use `sigsuspend` instead of `sleep` in `waitfg` to hang the foreground process.
2. In the handler for `SIGCHLD`, I adopt while loop to reap all child processes at one time.

## lab 7: malloc lab (ill solution)
Instructions for setup and evaluation are listed in `malloclab-handout/malloclab.pdf`.  
The official handout does not provide trace files. My results are tested on traces files cloned from this [repo](https://github.com/Deconx/CSAPP-Lab/tree/master/initial_labs/08_Malloc%20Lab/traces).

Below is the benchmark of my 64&32-clean solutions. One can checkout to the corresponding commit to see the code and result.  
Performance = (util + thru) / 100

|  setup      | Perf index |
| :-----:     | :--------: |
|  origin     |  -  + -    |
|  reference  |  44 + 24   |
|  opt ref    |  44 + 24   |
|  exp list   |  -  + -    |

- origin: simple implementation in the init handout (mdriver reports errors on this version).
- reference: implicitly free list analysed in the textbook (Section 9.9.12).
- opt ref: optimized implicitly free list with removing allocated blocks' footer (Section 9.9.11).
- exp list: explicitly free list. Free blocks are sorted by address. (Section 9.9.13).

Note: `exp list` has segmentation fault in several trace files, while it could pass the two short trace files. 
For example, the `amptjp-bal.rep` would report fault after invoking thousands of malloc and free. It's extremely tough to debug, so I have no choice but give up.

Section 9.9.14 discusses another technique, segregated storage. As no compound data types are allowed, we can save the lists of different block size into the begining of heap. Then, the operations are similar to `exp list` version.


## lab 8: proxy lab
Instructions are described in `proxylab-handout/proxylab.pdf`.  

### additional note
- curl is used to debug/test/autograde the solution.
However, the curl version in the wsl is not compatible with the `HTTP/1.0`, and it would report `Received HTTP/0.9 when not allowed`.
So I added `--http0.9` option in the `driver.sh` for autograding. During testing or debugging, one also needs to add this option in cmd line.

- `/usr/bin/python` does not exist in my wsl. I changed it into `/usr/bin/python3` in the `nop-server.py`.

- I introduced the `sbuf` lib from the offical code examples to implement the producer-consumer model for the concurrency version.

- I did not implement the cache, since I used producer-consumer model to implement the concurrency version. Basically, one can refer to Fig. 12-26 in Section 12.5.4 to implement the reader-writer model for the cache.