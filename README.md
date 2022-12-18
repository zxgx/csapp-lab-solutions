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
