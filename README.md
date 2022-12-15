# CSAPP lab

My solutions for the labs of [CSAPP 3rd edition](http://csapp.cs.cmu.edu/3e/labs.html)

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

