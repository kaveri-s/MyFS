cd temp
ls
mkdir hello
echo "Hello World" > hello.txt
ls -al
cd hello
ls -al
cd ..
cat hello.txt
echo "Hello Again" >> hello.txt
cat hello.txt
cp hello.txt hi.txt
ls -al
cd hello
mkdir onemore
ls -al
cp ../hello.txt onemore/hi2.txt
ls -l onemore/hi2.txt
rmdir hello
rm onemore/hi2.txt
rmdir onemore
rmdir hello
rm hello.txt
cd ..

