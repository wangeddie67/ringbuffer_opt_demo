
result_title = []
result_table = []

file_obj = open("autorun.out", "r", encoding="utf-8")

for line in file_obj:
    if line.startswith("===="):
        title = line[4:-5].strip()
        result_title.append(title)
        col = len(result_title)
        if len(result_table) > 1:
            while len(result_table[0]) < col + 1:
                for row in result_table:
                    row.append('0')

    elif line.startswith("Arguments"):
        tokens = line.split(" ")
        thread_num = int(tokens[2])
        while len(result_table) <= thread_num:
            result_table.append([str(thread_num)] + ['0'] * len(result_title))

    elif line.startswith("Valid operation speed"):
        tokens = line.split(" ")
        valid_speed = tokens[3]
        result_table[thread_num][col] = valid_speed

# Print table
print("," + ",".join(result_title))
for row in result_table:
    print(",".join(row))
