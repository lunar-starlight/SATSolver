for f in $(ls $1)
do
    time ./a.out < $1/$f
done
