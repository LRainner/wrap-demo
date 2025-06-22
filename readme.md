# gcc warp 编译选项demo

最近在公司的项目中发现了一个神奇问题，在Gitlab ci中，单元测试的结果可以写到文件中，并生成产物，但是在本地就不能写到文件中。

因为本地开发机内存较小，无法编译release版本的，一直编译的都是debug版本。之前也一直没有试过这个单元测试，这个单元测试通过fprintf将测试结果写到文件流中，看代码是没有一点问题，排查了半天发现，之前为了统一debug输出格式，通过gcc warp这个编译选项，将fprintf包装了起来，重新实现了fprintf逻辑，导致检测结果没有写到文件中，而是直接输出到stderr上。

wrap编译选项的作用是，在编译时，创建一个包装函数，拦截对应符号的调用，使其指向__wrap_对应前缀的符号，__real_对应前缀的符号指向原始真正的符号。

通过这个仓库的例子，就可以了解到具体的逻辑和用法了。

## warp 非内建函数
我们先进入foo文件夹中，foo文件夹中有一个foo函数，不会打印多余的东西，一个wrap函数，会在return之前打印出`[wrap foo]`

`gcc -g main.c wrap.c -Wl,--wrap=foo -o test_debug`

![](https://raw.githubusercontent.com/LRainner/Pic/main/img/839e5a042b471fa513a5a14d25010367.png)


`gcc -O2 main.c wrap.c -Wl,--wrap=foo -o test_foo_release`

![](https://raw.githubusercontent.com/LRainner/Pic/main/img/c6f3639f77d99be6386852a6682ff74b.png)

看起来是符合预期的，debug版和release版都成功调用了_wrap_foo函数，并且打印了`[wrap foo]`

到这里应该已经讲清楚了wrap的工作原理了，但是为什么还没有结束呢，因为这里还没有解决我的问题，内建函数的行为好像不完全是这样。

```
gcc -g main.c wrap.c -Wl,--wrap=foo -o test_debug

gcc -O2 main.c wrap.c -Wl,--wrap=foo -o test_release

gcc -g main.c wrap.h  -Wl,--wrap=fprintf -o wrap_fprintf_debug

gcc -O2 main.c wrap.h wrap.o  -Wl,--wrap=fprintf -o wrap_fprintf_release

gcc -c wrap.c -o wrap.o

gcc -g main.c wrap.c ../fprintf/wrap.h  ../fprintf/wrap.o -Wl,--wrap=foo,--wrap=fprintf -o test_debug

gcc -O2 main.c wrap.c ../fprintf/wrap.h  ../fprintf/wrap.o -U_FORTIFY_SOURCE -Wl,--wrap=foo -o test_release

```