# CSAPP lab

My solutions for the labs of [CSAPP 3rd edition](http://csapp.cs.cmu.edu/3e/labs.html)

Just for fun!

Below are the notes for each lab dir.

## lab 1: data lab
Google maybe helpful when get in trouble:)

中文参考：https://zhuanlan.zhihu.com/p/59534845

## lab 2: bomb lab
1. Use `objdump` to generate x86 asm code. 
```bash
cd lab/bomb
objdump -d ./bomb > bomb.asm
```

2. Read the `<phase_x>` (x = 1 - 6) segments in the `bomb.asm` file. Converting the asm code into c code might be helpful.

The ciphers are in the `cipher` file. Several phases have multiple solutions.

## lab 3: attack lab
under development
