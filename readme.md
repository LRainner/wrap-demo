# gcc warp 编译选项

最近在公司的项目中发现了一个神奇问题，在Gitlab ci中，单元测试的结果可以写到文件中，并生成产物，但是在本地就不能写到文件中。

因为本地开发机内存较小，无法编译release版本的，一直编译的都是debug版本。之前也一直没有单独跑过这个单元测试，这个单元测试通过fprintf将测试结果写到文件流中，看代码是没有一点问题，排查了半天发现，之前为了统一debug输出格式，通过gcc warp这个编译选项，将fprintf包装了起来，重新实现了fprintf逻辑，导致检测结果没有写到文件中，而是直接输出到stderr上。

wrap编译选项的作用是，在编译时，创建一个包装函数，拦截对应符号的调用，使其指向__wrap_对应前缀的符号，__real_对应前缀的符号指向原始真正的符号。

通过这个仓库的例子，就可以了解到具体的逻辑和用法了。

## warp 非内建函数
我们先进入foo文件夹中，foo文件夹中有一个foo函数，不会打印多余的东西，一个wrap函数，会在return之前打印出`[WRAP foo]`

`gcc -g main.c wrap.c -Wl,--wrap=foo -o test_debug`

![](https://raw.githubusercontent.com/LRainner/Pic/main/img/839e5a042b471fa513a5a14d25010367.png)


`gcc -O2 main.c wrap.c -Wl,--wrap=foo -o test_foo_release`

![](https://raw.githubusercontent.com/LRainner/Pic/main/img/c6f3639f77d99be6386852a6682ff74b.png)

看起来是符合预期的，debug版和release版都成功调用了_wrap_foo函数，并且打印了`[WRAP foo]`

到这里应该已经讲清楚了wrap的工作原理了，但是为什么还没有结束呢，因为这里还没有解决我的问题，内建函数的行为好像不完全是这样。

## wrap 内建函数
我们再进入fprintf文件，实现了__wrap_fprintf函数，在调用__real_fprintf之前构造新的字符串，在字符串前边拼接上`[WRAP] `，来看一下wrap内建函数的行为

`gcc -g main.c wrap.h wrap.c -Wl,--wrap=fprintf -o wrap_fprintf_debug`

![](https://raw.githubusercontent.com/LRainner/Pic/main/img/933da1541e313a864ec3b58e84ded528.png)

`gcc -O2 main.c wrap.h wrap.c -Wl,--wrap=fprintf -o wrap_fprintf_release`

![](https://raw.githubusercontent.com/LRainner/Pic/main/img/c1d887c7e0856077d7008c67c82995ff.png)

看起来debug版本的fprintf函数成功被wrap了，release版本的fprintf函数没有被成功wrap。看起来已经和ci上release版本和本地debug版本的单元测试的效果一样了。
这是为什么呢，为什么release版本的内建函数没有被成功wrap，我们再试一下编译一个静态库，链接到foo试试。

## wrap 静态库
我们先将fprintf文件夹中的wrap.c编译成静态库。

`gcc -c wrap.c -o wrap.o`

再将编译foo.c，wrap foo、fprintf，再将刚编译好的静态库链接上试试。

`gcc -g main.c wrap.c ../fprintf/wrap.h  ../fprintf/wrap.o -Wl,--wrap=foo,--wrap=fprintf -o wrap_foo_fprintf_debug`

![](https://raw.githubusercontent.com/LRainner/Pic/main/img/93357dfff0f130d97df1bb722a64359c.png)

`gcc -O2 main.c wrap.c ../fprintf/wrap.h  ../fprintf/wrap.o -Wl,--wrap=foo,--wrap=fprintf -o wrap_foo_fprintf_release`

![](https://raw.githubusercontent.com/LRainner/Pic/main/img/25952d34389787501ceec511d9d81b31.png)

看起来foo函数的debug版本和release版本都被成功wrap，而fprintf只在debug版本被成功wrap，release版本的fprintf依然没有被成功wrap，和上边直接编译release版本的效果是一样的。

我们通过IDA来反编译一下release版本的wrap_fprintf_release文件

![](https://raw.githubusercontent.com/LRainner/Pic/main/img/3879d81ae7d4e94b21b4141e3e9d77f7.png)

原来是在编译release版本时在编译阶段被优化成了__fprintf_chk，在链接阶段就不会链接__wrap_fprintf，而是链接__fprintf_chk，导致release版本的fprintf没有被成功wrap。

__fprintf_chk是glibc的内建函数，在使用O2或者更高的编译选项时，自动将fprintf替换成__fprintf_chk，增加额外的安全检查，可以通过`-U_FORTIFY_SOURCE`关闭这项优化，我们来试一下。

`gcc -O2 main.c wrap.c ../fprintf/wrap.h  ../fprintf/wrap.o -U_FORTIFY_SOURCE -Wl,--wrap=foo,--wrap=fprintf -o test_wrap_foo_fprintf_release`

![](https://raw.githubusercontent.com/LRainner/Pic/main/img/099133ae0a85f6c273bd2766caf95446.png)

这样release版本的内建函数也被成功wrap了，一般是不推荐使用`-U_FORTIFY_SOURCE`编译参数，会关闭很多安全检查，导致缓冲区溢出、格式化字符串攻击等漏洞。
