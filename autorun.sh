
make

rm autorun.out

for i in $(seq 2 36); do
    # ./bin/mutex_blkring -n "$i" -l 256 -o 10000 | tee -a autorun.out
    # ./bin/mutex_nonblkring -n "$i" -l 256 -o 10000 | tee -a autorun.out
    # ./bin/atomic_blkring -n "$i" -l 256 -o 100000 | tee -a autorun.out
    ./bin/align_blkring -n "$i" -l 256 -o 100000 | tee -a autorun.out
    # ./bin/p64_blkring -n "$i" -l 256 -o 100000 | tee -a autorun.out
done

python3 collect_result.py
