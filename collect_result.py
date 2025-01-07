
result_table = []

file_obj = open("autorun.out", "r", encoding="utf-8")

for line in file_obj:
    if line.startswith("Require"):
        tokens = line.split(" ")
        thread_num = tokens[1]
        result_table.append([thread_num, 0, 0])
    elif line.startswith("Total operation speed"):
        tokens = line.split(" ")
        total_speed = tokens[3]
        result_table[-1][1] = total_speed
    elif line.startswith("Valid operation speed"):
        tokens = line.split(" ")
        total_speed = tokens[3]
        result_table[-1][2] = total_speed

for thread_num, total_speed, valid_speed in result_table:
    print(f"{thread_num}\t{total_speed}\t{valid_speed}")
