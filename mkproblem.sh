#!/bin/zsh

problemname='experiment6'
python mkdata.py < mkdata.txt > "${problemname}"

dirname="QLKP/${problemname}"
mkdir "${dirname}"
echo "10, 10000" > "${dirname}/problem_params"

for i in {0..9}
{
mkdir "${dirname}/group${i}";
#echo $i;
echo $(((2**$i)*100)) > "${dirname}/group${i}/items" ;
#cat "${dirname}/group${i}/items";
sed 1d "${problemname}" >> "${dirname}/group${i}/items" ;
echo "f\n 1,5" > "${dirname}/group${i}/func" ;
}

rm "$problemname"

