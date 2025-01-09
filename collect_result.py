
result_table = []

file_obj = open("autorun.out", "r", encoding="utf-8")

for line in file_obj:
    if line.startswith("Require"):
        tokens = line.split(" ")
        thread_num = tokens[1]
        result_table.append([thread_num, 0])
    elif line.startswith("Valid operation speed"):
        tokens = line.split(" ")
        valid_speed = tokens[3]
        result_table[-1][1] = valid_speed

for thread_num, valid_speed in result_table:
    print(f"{thread_num}\t{valid_speed}")
