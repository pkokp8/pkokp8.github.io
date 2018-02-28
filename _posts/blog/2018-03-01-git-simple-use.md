---
layout:     post
title: git的简单操作
description: 记录我从零开始学git的一些命令
category: blog
---
<p>
  记录一下git的使用。系统是ubuntu16.04. 安装好git环境  
  首先在github上右上角点一下+，然后New repository。  
  也可以本地创建好后push上去，不过这样也挺方便的。初学者嘛
</p>

<p>
  然后是clone一个master，例如刚才创建的pkokp8.github.io.git分支
</p>
<pre>
# git clone https://github.com/pkokp8/pkokp8.github.io.git
</pre>

<p>接着就可以修改、提交文件</p>
<pre>
# echo "# test info" >> README.md           #新建README.md
# git add .                                 #本地仓库新增这个文件
# git commit -m "commit the readme"         #提交到本地，日志为-m后面""内的字符串
# git push -u origin master                 #push到远程主线。此时可以使用reset来回退代码，这样远端就不会有这次commit的记录
</pre>

<p>
  此时可能因为没有用户名和密码，会提示错误。可以配置后再执行上述命令
</p>

<pre>
# git config --global user.name "pkokp8"
# git config --global user.email "xxx@xx.com"
</pre>

<p>
  查看本地的提交记录，简单的可以使用。配合reset可以回退版本。注意-hard会把本地修改覆盖掉。  
  类似svn的svn revert ./ -R;svn up -r xxx
</p>
<pre>
# git log
commit af17a9f424c5da5b4b06f90e69eb069f89124e40
Author: pkokp8 <xxx@xx.com>
Date:   Thu Mar 1 00:23:55 2018 +0800

    modify readme

# git reset -hard af17a9f424c5da5b4b06f90e69eb069f89124e40 
</pre>


<p>
  接着就可以创建分支了：
</p>
<pre>
# git branch firstversion                   #创建名为firstversion的分支
# git checkout firstversion                 #切换到这个分支
# git push -u origin firstversion           #把这个分支push到远端
# git checkout master                       #切换回主线
# git branch                                #查看当前处于哪条分支（看*），以及存在哪些分支
  firstversion
* master
# git diff                                  #查看本地与本地仓库之间的差异
</pre>



<p>
  暂时就用到这么多。我感觉我还是要学习一个
</p>