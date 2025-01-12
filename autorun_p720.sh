
make

rm autorun.out

MAX_THREAD=64

# echo "==== mutex (numa1) ====" >> autorun.out
# for i in $(seq 2 $MAX_THREAD); do
#     ./bin/mutex_blkring -n "$i" -l 256 -o 10000 -m numa1 | tee -a autorun.out
# done

# echo "==== lockfree (numa1) ====" >> autorun.out
# for i in $(seq 2 $MAX_THREAD); do
#     ./bin/lockfree_blkring -n "$i" -l 256 -o 100000 -m numa1 | tee -a autorun.out
# done

# echo "==== atomic (numa1) ====" >> autorun.out
# for i in $(seq 2 $MAX_THREAD); do
#     ./bin/atomic_blkring -n "$i" -l 256 -o 100000 -m numa1 | tee -a autorun.out
# done

# echo "==== align (numa1) ====" >> autorun.out
# for i in $(seq 2 $MAX_THREAD); do
#     ./bin/align_blkring -n "$i" -l 256 -o 100000 -m numa1 | tee -a autorun.out
# done

echo "==== buck (numa1) ====" >> autorun.out
for i in $(seq 2 $MAX_THREAD); do
    ./bin/buck_blkring -n "$i" -l 256 -o 100000 -m numa1 | tee -a autorun.out
done

# echo "==== mutex (numa2) ====" >> autorun.out
# for i in $(seq 2 $MAX_THREAD); do
#     ./bin/mutex_blkring -n "$i" -l 256 -o 10000 -m numa2 | tee -a autorun.out
# done

# echo "==== lockfree (numa2) ====" >> autorun.out
# for i in $(seq 2 $MAX_THREAD); do
#     ./bin/lockfree_blkring -n "$i" -l 256 -o 100000 -m numa2 | tee -a autorun.out
# done

# echo "==== atomic (numa2) ====" >> autorun.out
# for i in $(seq 2 $MAX_THREAD); do
#     ./bin/atomic_blkring -n "$i" -l 256 -o 100000 -m numa2 | tee -a autorun.out
# done

# echo "==== align (numa2) ====" >> autorun.out
# for i in $(seq 2 $MAX_THREAD); do
#     ./bin/align_blkring -n "$i" -l 256 -o 100000 -m numa2 | tee -a autorun.out
# done

# echo "==== buck (numa2) ====" >> autorun.out
# for i in $(seq 2 $MAX_THREAD); do
#     ./bin/buck_blkring -n "$i" -l 256 -o 100000 -m numa2 | tee -a autorun.out
# done

python3 collect_result.py
