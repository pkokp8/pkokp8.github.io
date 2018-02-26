---
layout: post
title: 复制一个list
date: 2018-02-23
categories: python
tags: [python,list]
description: python如何复制一个list。
---
<p>初学python，爬图片的时候想复制一个list。一开始以为可以直接用这种方式复制list：
</p>

<pre>
lista=[xxx, yyy, zzz]
listb=lista
</pre>

<p>后来发现修改listb后lista的值也被修改了。原因是实际上listb就是lista。</p>
<p>可以尝试用id函数打印出两个list的内存地址:</p>
<pre>
lista = [0, 1, 2, 3, 4]
print("%s %s" % (lista, id(lista)))

listb = lista
print("%s %s" % (listb, id(listb)))

=============================
[0, 1, 2, 3, 4] 8246256
[0, 1, 2, 3, 4] 8246256

</pre>

<p>
两个list是同一个，这肯定不行。目的就是为了新建一个list，并且修改新建的list不能影响之前的list    
</p>
<p>
一种办法很粗暴：用for循环遍历旧的list，新建一个空list，然后通过append方法把每一个元素添加进新的list。于是代码就是：
</p>
<pre>
listnew=[]
for i in listold:
    listnew.append(i)
</pre>
<p>
这个类似于c语言中写出这样的代码:
</p>
<pre>
char old[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
char new[10];
int i = 0;
for(i = 0;i < sizeof(old)/sizeof(char), i++)
{
	new[i] = old[i];
}
</pre>
<p>你退群吧。除非要对每个变量进行处理（例如对一张yuv图像进行转换成rgb图像，就需要遍历数组的所有变量），否则遍历有什么意义？memcpy()不行吗</p>
<p>python有很多巧妙方法，例如：</p>
<pre>
listnew = listold*1
</pre>
<p>当然，还有很多方法，这里举点例子：</p>
<pre>
lista = [0, 1, 2, 3, 4]
print("a %s %s" % (lista, id(lista)))

lista_ = lista					#实际没有复制
print("a_ %s %s" % (lista_, id(lista_)))

'''
以下是复制
'''

listb = lista*1
print("b %s %s" % (listb, id(listb)))

listc=[]
for i in lista:
    listc.append(i)				#append方法
print("c %s %s" % (listc, id(listc)))

listd=[]
listd=lista[:] 					#slice
print("d %s %s" % (listd, id(listd)))

import copy
liste = copy.copy(lista)
print("e %s %s" % (liste, id(liste)))

listf = [i for i in lista]		#列表生成式
print("f %s %s" % (listf, id(listf)))


========================================
a [0, 1, 2, 3, 4] 40817648
a_ [0, 1, 2, 3, 4] 40817648
b [0, 1, 2, 3, 4] 40735088
c [0, 1, 2, 3, 4] 40817568
d [0, 1, 2, 3, 4] 40818648
e [0, 1, 2, 3, 4] 40818008
f [0, 1, 2, 3, 4] 40819608
</pre>

此外，list中带有list，此时list就涉及了深拷贝和浅拷贝。不过这次先不考虑这个。还没到需要写list中带list的代码。用到了再学吧。