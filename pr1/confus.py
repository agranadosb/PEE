import sys

file = sys.argv[1]
lines = []
with open(file) as f:
    lines = f.readlines()

symbs = set()
res = {}
for i in lines:
    value = tuple(i.rstrip().split())
    symbs.add(value[0])
    symbs.add(value[1])
    res[value] = res.get(value, 0) + 1

symbs = sorted(list(symbs))
print("symb ", end=" ")
for i in symbs + ['error']:
    print(f"{i:^5s}", end=" ")

print()
for i in symbs:
    print(f"{i:5}", end=" ")
    total = 0
    for j in symbs:
        total += res[(i, j)]
    for j in symbs:
        print(f"{res[(i, j)]:^5}", end=' ')

        if i == j:
            error = 1 - res[(i, j)] / total
    print(str(round(error * 100, 1)) + "%")