规则：
1.xls的子表单名字中不应该有"_"
2.每个item的第一个key统一为"id"
3.xls先导出为json（可以用cocostudio的数据编辑器把xls导出为json)，再用此程序转换
4.有value为table的filed，key应该写成以"_tab"结尾，比如key为pos_tab，value直接用lua的语法配置，比如{x=10,y=20}
5.xls详细用法，参考 test
6.打开windows控制台，命令：
    json2lua.exe testJ2L.json 回车，则会生成testJ2L.lua文件。

更加详细内容，参考代码。