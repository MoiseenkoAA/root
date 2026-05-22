
dir=`pwd`
for i in `ls` ToolsLib/wwwcgi ; do
echo $i
if [ -d $dir/$i ]; then
echo $i
cd $dir/$i
pwd

for file in `ls *.cpp *.h` ; do
echo $file
replace.out $file CTOO CMaa
replace.out $file CMaaFile2 CMaaFile
done

cd $dir
fi
done
