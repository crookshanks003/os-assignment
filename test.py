f_soln = open("soln.txt", "r")
f_que = open("thousand.txt", "r")

soln = f_soln.read();
que = f_que.read()

arr_soln = soln.split(" ")
arr_que = que.split(" ")

def get_duplicates():
    print("-------duplicates-------")
    seen = set()
    dupe = [x for x in arr_soln if x in seen or seen.add(x)]

    for x in dupe:
        print(x)

    return len(dupe)

def get_missing():
    print("-------missing-------")
    set1 = set(arr_que)
    set2 = set(arr_soln)

    missing = list(sorted(set1 - set2))
    for i in missing: 
        print(i)

    return len(missing)

print("count -- ", len(arr_soln)-1)
print("total -- ", get_duplicates())
print("total -- ", get_missing())
