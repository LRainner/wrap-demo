
```
gcc -g main.c wrap.c -Wl,--wrap=foo -o test_debug

gcc -O2 main.c wrap.c -Wl,--wrap=foo -o test_release

gcc -g main.c wrap.h  -Wl,--wrap=fprintf -o wrap_fprintf_debug

gcc -O2 main.c wrap.h wrap.o  -Wl,--wrap=fprintf -o wrap_fprintf_release

gcc -c wrap.c -o wrap.o

gcc -g main.c wrap.c ../fprintf/wrap.h  ../fprintf/wrap.o -Wl,--wrap=foo,--wrap=fprintf -o test_debug

gcc -O2 main.c wrap.c ../fprintf/wrap.h  ../fprintf/wrap.o -U_FORTIFY_SOURCE -Wl,--wrap=foo -o test_release

```