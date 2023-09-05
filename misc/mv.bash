list=`ls`

for f in $list
do
if [ -f "$f" ];
then
mv $f ./misc
fi
done