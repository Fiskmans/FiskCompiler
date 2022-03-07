# FiskCompiler

There are better tools out there, use them

This is my little experiment to see if i'm capable of making a compiler thats able to at least correctly compile a small test program to AMD64 and then link it

```cpp
int main ()
{
}
```
should compile down to

```asm
xor eax, eax
retq
```
