
make

rm autorun.out

for i in $(seq 2 64); do
    echo ./UUT -n $i -l 256 | tee -a autorun.out
    # ./mutex_blkring -n "$i" -l 400 | tee -a autorun.out
    # ./mutex_nonblkring -n "$i" -l 400 | tee -a autorun.out
    ./atomic_blkring -n "$i" -l 256 | tee -a autorun.out
done
