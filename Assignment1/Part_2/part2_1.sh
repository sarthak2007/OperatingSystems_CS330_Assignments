#! /bin/sh

gcc part2.c

# # Part1
# ./a.out @ Kanpur IITK > a.txt
# grep -rF Kanpur IITK | wc -l > b.txt
# diff a.txt b.txt
# rm a.txt b.txt

# ./a.out @ Engineering IITK/Postgraduate/details.txt > a.txt
# grep -rF Engineering IITK/Postgraduate/details.txt | wc -l > b.txt
# diff a.txt b.txt
# rm a.txt b.txt

# ./a.out @ Engineering IITK/Undergraduate/ > a.txt
# grep -rF Engineering IITK/Undergraduate/ | wc -l > b.txt
# diff a.txt b.txt
# rm a.txt b.txt

# # Part2
# ./a.out $ Kanpur IITK c.txt wc -l > a.txt
# grep -rF Kanpur IITK |tee d.txt | wc -l > b.txt
# diff a.txt b.txt
# diff c.txt d.txt 

# ./a.out $ Kanpur IITK c.txt sort > a.txt
# grep -rF Kanpur IITK |tee d.txt | sort > b.txt
# diff a.txt b.txt
# diff c.txt d.txt 

# ./a.out $ Kanpur IITK c.txt sort -r > a.txt
# grep -rF Kanpur IITK |tee d.txt | sort -r > b.txt
# diff a.txt b.txt
# diff c.txt d.txt 

# ./a.out $ Kanpur IITK c.txt wc -m -c > a.txt
# grep -rF Kanpur IITK |tee d.txt | wc -m -c > b.txt
# diff a.txt b.txt
# diff c.txt d.txt 

# ./a.out $ Engineering IITK/Postgraduate/details.txt c.txt wc -l > a.txt
# grep -rF Engineering IITK/Postgraduate/details.txt |tee d.txt | wc -l > b.txt
# diff a.txt b.txt
# diff c.txt d.txt 

# ./a.out $ Engineering IITK/Postgraduate/details.txt c.txt sort > a.txt
# grep -rF Engineering IITK/Postgraduate/details.txt |tee d.txt | sort > b.txt
# diff a.txt b.txt
# diff c.txt d.txt 

# ./a.out $ Engineering IITK/Postgraduate/details.txt c.txt sort -r > a.txt
# grep -rF Engineering IITK/Postgraduate/details.txt |tee d.txt | sort -r > b.txt
# diff a.txt b.txt
# diff c.txt d.txt 

# ./a.out $ Engineering IITK/Postgraduate/details.txt c.txt wc -m -c > a.txt
# grep -rF Engineering IITK/Postgraduate/details.txt |tee d.txt | wc -m -c > b.txt
# diff a.txt b.txt
# diff c.txt d.txt

# ./a.out $ Engineering IITK/Undergraduate/ c.txt wc -l > a.txt
# grep -rF Engineering IITK/Undergraduate/ |tee d.txt | wc -l > b.txt
# diff a.txt b.txt
# diff c.txt d.txt 

# ./a.out $ Engineering IITK/Undergraduate/ c.txt sort > a.txt
# grep -rF Engineering IITK/Undergraduate/ |tee d.txt | sort > b.txt
# diff a.txt b.txt
# diff c.txt d.txt 

# ./a.out $ Engineering IITK/Undergraduate/ c.txt sort -r > a.txt
# grep -rF Engineering IITK/Undergraduate/ |tee d.txt | sort -r > b.txt
# diff a.txt b.txt
# diff c.txt d.txt 

# ./a.out $ Engineering IITK/Undergraduate/ c.txt wc -m -c > a.txt
# grep -rF Engineering IITK/Undergraduate/ |tee d.txt | wc -m -c > b.txt
# diff a.txt b.txt
# diff c.txt d.txt

# Start
./a.out @ $1 $2 > a.txt
grep -rF $1 $2 | wc -l > b.txt
diff a.txt b.txt
rm a.txt b.txt

./a.out $ $1 $2 c.txt wc -l > a.txt
grep -rF $1 $2 |tee d.txt | wc -l > b.txt
diff a.txt b.txt
diff c.txt d.txt 

./a.out $ $1 $2 c.txt sort > a.txt
grep -rF $1 $2 |tee d.txt | sort > b.txt
diff a.txt b.txt
diff c.txt d.txt 

./a.out $ $1 $2 c.txt sort -r > a.txt
grep -rF $1 $2 |tee d.txt | sort -r > b.txt
diff a.txt b.txt
diff c.txt d.txt 

./a.out $ $1 $2 c.txt wc -m -c > a.txt
grep -rF $1 $2 |tee d.txt | wc -m -c > b.txt
diff a.txt b.txt
diff c.txt d.txt
# End