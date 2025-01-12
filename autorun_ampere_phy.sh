
make
make arm

rm autorun.out

MAX_THREAD=16

echo "==== mutex ====" >> autorun.out
for i in $(seq 2 $MAX_THREAD); do
    ./bin/mutex_blkring -n "$i" -l 256 -o 10000 -m numa1-phy | tee -a autorun.out
done

echo "==== lockfree ====" >> autorun.out
for i in $(seq 2 $MAX_THREAD); do
    ./bin/lockfree_blkring -n "$i" -l 256 -o 100000 -m numa1-phy | tee -a autorun.out
done

echo "==== atomic ====" >> autorun.out
for i in $(seq 2 $MAX_THREAD); do
    ./bin/atomic_blkring -n "$i" -l 256 -o 100000 -m numa1-phy | tee -a autorun.out
done

echo "==== align ====" >> autorun.out
for i in $(seq 2 $MAX_THREAD); do
    ./bin/align_blkring -n "$i" -l 256 -o 100000 -m numa1-phy | tee -a autorun.out
done

echo "==== buck ====" >> autorun.out
for i in $(seq 2 $MAX_THREAD); do
    ./bin/buck_blkring -n "$i" -l 256 -o 100000 -m numa1-phy | tee -a autorun.out
done

echo "==== pc64 ====" >> autorun.out
for i in $(seq 2 $MAX_THREAD); do
    ./bin/p64_blkring -n "$i" -l 256 -o 100000 -m numa1-phy | tee -a autorun.out
done

python3 collect_result.py
