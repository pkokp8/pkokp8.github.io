---
layout:     post
title: source和export
description: shell脚本中何时用source，何时用export？
category: blog
---
## 现象 ##

# test.sh中将ABC赋值为1 #
./test.sh之后echo $ABC的内容为空  
source test.sh之后echo $ABC的内容为1  

# 在当前shell下赋值 #
ABC=999  
export AAA=998  
在test.sh中将ABC和AAA都echo出来，得到的结果是  
ABC为空，AAA为998  



## 原因 ##
1.当我们执行脚本的时候，是当前终端所在的shell fork一个子shell然后执行，执行完了再返回终端所在的shell。  
2.一个shell中的系统环境变量只对该shell或者它的子shell有效，该shell结束时变量消失（类似C语言的作用域）  
3.source是在当前shell中执行  



## 结论 ##
1.export定义的变量只在当前shell下有效，对子shell是无效的。  
2.脚本中定义变量，用source去执行，可以影响父shell  

## 通俗地讲 ##
直接赋值影响当前shell  
source可以影响父shell下的所有子shell、本身、本身的所有子shell  
export只能通过影响自己、本身的子shell  