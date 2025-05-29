import gurobipy as gp
from gurobipy import GRB
import pandas as pd

# read data
data = pd.read_csv("data.csv", index_col=0)
objects = data.index.tolist()
I = list(range(len(objects)))  # star No.
n = 448  # total time
J = list(range(n))  # time
L = list(range(1))  # telescope 

# model params
Ti = {i: 20 for i in I}  # observation time
u = {i: 1.0 for i in I}  # star weight
S = 5                   # cold time
theta_a = 10            # angle
theta_m = 20            # angle to moon
U = len(I)              # upper bound

# 建立高度角 a_ij 與月亮角度 m_ij 資料
a = {(i, j): 90 if data.iloc[i]["object_rise"] <= j <= data.iloc[i]["object_set"] else 0 for i in I for j in J}
m = {(i, j): 90 if data.iloc[i]["moon_near"] == -1 or j < data.iloc[i]["moon_near"] or j > data.iloc[i]["moon_leave"] else 0 for i in I for j in J}

# model
model = gp.Model("Telescope_Schedule")

# 決策變數
x = model.addVars(I, J, L, vtype=GRB.BINARY, name="x")
y = model.addVars(I, J, vtype=GRB.BINARY, name="y")

# Objective
model.setObjective(gp.quicksum(u[i] * x[i, j, l] for i in I for j in J for l in L), GRB.MAXIMIZE)

# Constraint 1: 每顆星最多只能觀一次
for i in I:
    model.addConstr(gp.quicksum(x[i, j, l] for j in J for l in L) <= 1)

# Constraint 2: 每個時段每台望遠鏡最多只觀一顆星
for j in J:
    for l in L:
        model.addConstr(gp.quicksum(x[i, j, l] for i in I) <= 1)

# Constraint 3: 若有安排觀測，對應時段的 y 值需 >= Ti
for i in I:
    for j in J:
        for l in L:
            if j + Ti[i] - 1 < n:
                model.addConstr(gp.quicksum(y[i, k] for k in range(j, j + Ti[i])) >= Ti[i] * x[i, j, l])

# Constraint 4: 高度角需超過門檻
for i in I:
    for j in J:
        model.addConstr(y[i, j] * (a[i, j] - theta_a) >= 0)

# Constraint 5: 離月亮角度需超過門檻
for i in I:
    for j in J:
        model.addConstr(y[i, j] * (m[i, j] - theta_m) >= 0)

# Constraint 6: 快接近尾端不能開始觀測
for i in I:
    for k in range(n - Ti[i] + 1, n):
        for l in L:
            model.addConstr(x[i, k, l] == 0)

# Constraint 7: 緩衝時間：其他星體不能在同段望遠鏡上安排
for i in I:
    for j in J:
        if j + Ti[i] + S - 1 < n:
            for l in L:
                model.addConstr(
                    gp.quicksum(x[r, k, l] for r in I for k in range(j, j + Ti[i] + S)) <= U * (1 - x[i, j, l])
                )

# run
model.optimize()

# reults
if model.status == GRB.OPTIMAL:
    for i, j, l in x.keys():
        if x[i, j, l].X > 0.5:
            print(f"Observe object {objects[i]} at time {j} using telescope {l}")
else:
    print("No optimal solution found.")
