import random

var_cnt = 500
clause_cnt = 100000

gens = [random.random() for i in range(clause_cnt)]
gens = [0] + gens

print('c probability for each')
print('c', end=" ")

for i in range(1, var_cnt + 1):
    print(str(i) + "=>" + str(round(100 * gens[i], 2)) + "%", end=" ")

print("\np cnf", var_cnt, clause_cnt)

def negator():
    if random.random() > 0.75:
        return -1
    return 1

def printer(var_cnt: int, sez: list):
    if len(sez) < 2:
        sez = random.sample(range(1,var_cnt+1), k=random.randint(3, var_cnt))
    for x in sez:
        print(negator()*x, end=" ")
    
    print(0)


for i in range(clause_cnt):
    temp = []
    for j in range(1, var_cnt + 1):
        if random.random() < gens[j]:
            temp.append(j)
    random.shuffle(temp)
    printer(var_cnt, temp)
