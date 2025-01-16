这是网友移植的OpenGL库，但似乎没GitHub工程，我留下来做备份，原地址：https://cloud.tencent.com/developer/article/1934541

在ESP32上移植OpenGL实现（一）
发布于 2022-01-14 16:37:36
2.6K0
举报
文章被收录于专栏：
KAAAsS's Blog
看@FrostMiku最近一直在玩ESP32，而且看起来真的很有趣，所以就求了个链接买了一块板子自己玩。咱也很想玩玩嵌入式嘛。不过ESP32的板子倒是真便宜，基本都在二三十左右。我这块由于带了个TFT屏，所以稍贵，价格是38。到手之后发现屏幕虽然不大，但是分辨率有135×240，所以整体显示效果还是很清晰的。正好最近在学OpenGL，于是就觉得移植一个OpenGL实现玩玩。

选择实现
我还没自己实现OpenGL的功力，所以还是用别人吧。大致找到了如下实现：

Google的SwiftShader。其实原本不是Google的，前身是TransGaming的开源项目swShader（后转为闭源），不过后来由Google接盘再开源的。SwiftShader实现了Vulkan、OpenGL ES、D3D 9，并且运行效率很不错。不过SwiftShader大量使用多线程，显然不适合ESP32。
Mesa。Mesa大概是最被广泛使用的OpenGL/Vulkan的软件实现了，Mesa的运行销量也相当不错。但是Mesa过于庞大，移植难度非常大。
Vincent（ogles）。Vincent实现了OpenGL ES 1.1，由C++编写，本身就是为嵌入式打造的。不过我简单浏览了下，为了优化，Vincent的很多逻辑都是直接内嵌汇编的，和平台关系过于紧密，移植起来还是有难度的。
TinyGL。TinyGL是Fabrice Bellard开发的OpenGL 1.1子集。Fabrice不用多说，是神仙级程序员。TinyGL是他开发的一个轻量级C语言的OpenGL软件实现。TinyGL的一大优点是，本身实现是纯C的，没有用到任何汇编内嵌，而且编译结果按官方说明只有40K，非常适合移植。
经过评估，我最后选择了TinyGL的一个分支实现PicoGL。PicoGL基于TinyGL 4.0，增加了直接写Linux Framebuffer的backend、使用Makefile组织项目、增加了定点数运算支持。

再开发：RepicoGL
不过对于移植来说，PicoGL还是有很多问题的。首先就是PicoGL增加的backend是写Linux Framebuffer的，然而ESP32并不是Embed Linux，所以要新编写一个直接写入内存Framebuffer的后端。其次就是改用更现代的CMake来控制编译流程。另外，我在试验过程中发现，现有的X11 backend的支持实际上是有问题的，最终的渲染结果会显示两份并且颜色也不对。而且，似乎内部渲染修改为RGB24时也无法给出正确的输出（默认是RGB565）。

因此，我在PicoGL的基础上又重新开发了一个backend。不过这个backend由于其特殊性，需要兼容各种不同的输入，所以原有的接口是无法满足开发需求的，因此还需要扩充若干函数。另外，由于我的开发环境是Arduino，因此还需要为C++的兼容做一些处理。

另外，SDL的backend还是可用的，因此可以用作图形程序的调试。不过SDL目前backend默认使用的bbp为8（在tk.c里可以调整）。


由于各处都有代码改动，所以干脆就另开一个RepicoGL项目好啦。代码整理完毕后，我应该会开一个repo上传的，时间大概在近期（咕）。

移植
因为实在是没有嵌入式开发经验，所以我选择了Arduino进行开发。直接上手esp-idf之类的还是有点顶不住。因此需要把RepicoGL做成一个库，不过我不咋熟悉Arduino，所以直接暴力的把所有文件丢到了一起（

屏幕显示用的是TFT_eSPI这个库。不过直接烧写发现程序运行错误，不断重启。通过coredump发现是内部绘制用zbuffer的像素buffer没有成功分配……后来发现，Arduino的ESP32环境下似乎不能一次性分配太大的内存？？？

因此只能把屏幕改小，这下是可以绘制了，但是绘制结果的颜色完全偏色……后来发现问题出在我传入Framebuffer数据的时候用的是uint8_t，用bpp8模式输出，然后两个程序的颜色表不同。换成uint16_t使用RGB565输出就能正确绘制颜色了。

另外参考一处测试（见Reference），ESP32的double运算性能较差，而且似乎并不是使用FPU，而是采用软件计算的，因此最好是让程序内部使用float进行运算。好在PicoGL使用了统一的定点数运算库，所以有一个统一的数据类型来负责计算，全都改为float就可以了。修改为float之后，输出帧率有了肉眼可见的提升。


实际帧率是大于30的，不过为了防止gif过大所以就进行了压缩。

然而由于开不了过大的存储空间，并且TinyGL内部是先将材质规格化到256×256再进行处理的，要开256*256*2的空间，所以材质暂时没有办法使用。另外还有一个机器人示例，但是由于glu和部分函数操作需要开缓存（当然也开不下），所以也没办法绘制所有部分。严格来说，只能画出来这么多：


嘛……至少也是正确的画出来了。下一步的移植重点（如果有的话）就是对暂时不能运行的函数尝试修正，并且继续整理RepicoGL了。

目前的代码如下，增加了很多奇怪的调试语句，之后应该会全都去掉的（逃

Arduino库：RepicoGL_arduino_v0.1.zip
 齿轮示例：gear_sample.zip

如果不能下载，请尝试“右键”-“链接另存为”。

Reference
OpenMoko的OpenGL/ES實做（http://orzlab.blogspot.com/2007/05/openmokoopengles.html）
ESP32 floating-point performance（https://blog.classycode.com/esp32-floating-point-performance-6e9f6f567a69）