import gurobipy as gp
from gurobipy import GRB
import pandas as pd

# Load data
data = pd.read_csv("../data/data.csv", index_col=0)
objects = data.index.tolist()
I = list(range(len(objects)))
n = 448
J = list(range(n))
L = list(range(1))  # You can change number of telescopes here

# Parameters

# fix obs time
# Ti = {i: 20 for i in I} 

Ti_values = [
    30, 30, 30, 20, 20, 20, 20, 30, 30, 30, 30, 30, 20, 30, 30, 30, 30, 30, 30, 40, 
    30, 20, 30, 20, 30, 30, 30, 30, 30, 30, 20, 30, 20, 20, 20, 30, 30, 30, 20, 40,
    20, 20, 40, 20, 20, 30, 20, 20, 30, 30, 30, 30, 30, 30, 30, 30, 30, 40, 40, 30, 
    40, 30, 30, 30, 40, 30, 30, 30, 30, 30, 30, 40, 40, 40, 30, 60, 30, 30, 30, 30,
    30, 30, 30, 40, 40, 30, 30, 40, 40, 40, 60, 30, 30, 30, 40, 40, 40, 60, 40, 40,
    30, 40, 30, 30, 40, 30, 30, 60, 40, 30
]
Ti = {i: Ti_values[i] for i in range(len(Ti_values))}

# fix rewards
# u = {i: 1.0 for i in I}
u_values = [
    3.0, 2.0, 2.0, 4.0, 1.0, 2.0, 2.0, 3.0, 3.0, 3.0, 1.0, 3.0, 1.0, 3.0, 2.0, 2.0, 2.0, 3.0, 3.0, 5.0,
    3.0, 1.0, 2.0, 3.0, 2.0, 3.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 4.0, 2.0, 1.0, 2.0, 2.0, 2.0, 2.0, 2.0,
    2.0, 1.0, 2.0, 2.0, 1.0, 2.0, 2.0, 2.0, 3.0, 2.0, 3.0, 2.0, 2.0, 3.0, 4.0, 3.0, 2.0, 3.0, 4.0, 3.0,
    4.0, 3.0, 3.0, 3.0, 3.0, 2.0, 2.0, 5.0, 3.0, 3.0, 3.0, 3.0, 4.0, 5.0, 3.0, 3.0, 3.0, 3.0, 3.0, 2.0,
    2.0, 2.0, 4.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 4.0, 5.0, 2.0, 2.0, 3.0, 4.0, 3.0, 5.0, 5.0, 4.0, 5.0,
    5.0, 4.0, 2.0, 2.0, 3.0, 4.0, 4.0, 4.0, 4.0, 4.0
]
u = {i: u_values[i] for i in range(len(u_values))}
S = 5
theta_a = 10
theta_m = 20
U = len(I)

# Create visibility parameters
a = {(i, j): 90 if data.iloc[i]["object_rise"] <= j <= data.iloc[i]["object_set"] else 0 for i in I for j in J}
m = {(i, j): 90 if data.iloc[i]["moon_near"] == -1 or j < data.iloc[i]["moon_near"] or j > data.iloc[i]["moon_leave"] else 0 for i in I for j in J}

# Model
model = gp.Model("Telescope_Scheduling")
x = model.addVars(I, J, L, vtype=GRB.BINARY, name="x")
y = model.addVars(I, J, vtype=GRB.BINARY, name="y")

# Objective
model.setObjective(gp.quicksum(u[i] * x[i, j, l] for i in I for j in J for l in L), GRB.MAXIMIZE)

# Constraint 1
for i in I:
    model.addConstr(gp.quicksum(x[i, j, l] for j in J for l in L) <= 1)

# Constraint 2
for j in J:
    for l in L:
        model.addConstr(gp.quicksum(x[i, j, l] for i in I) <= 1)

# Constraint 3
for i in I:
    for j in J:
        for l in L:
            if j + Ti[i] - 1 < n:
                model.addConstr(gp.quicksum(y[i, k] for k in range(j, j + Ti[i])) >= Ti[i] * x[i, j, l])

# Constraint 4
for i in I:
    for j in J:
        model.addConstr(y[i, j] * (a[i, j] - theta_a) >= 0)

# Constraint 5
for i in I:
    for j in J:
        model.addConstr(y[i, j] * (m[i, j] - theta_m) >= 0)

# Constraint 6
for i in I:
    for k in range(n - Ti[i] + 1, n):
        for l in L:
            model.addConstr(x[i, k, l] == 0)

# # Constraint 7 (Revised buffer exclusion)
# for i in I:
#     for j in J:
#         if j + Ti[i] + S - 1 < n:
#             for l in L:
#                 for r in I:
#                     if r != i:
#                         for k in range(j, j + Ti[i] + S):
#                             model.addConstr(x[r, k, l] <= 1 - x[i, j, l])
# Constraint 7
for j in J:
    for l in L:
        model.addConstr(
            gp.quicksum(
                x[i, t, l]
                for i in I
                for t in range(max(0, j - Ti[i] - S + 1), min(j + 1, n - Ti[i] + 1))
                if t + Ti[i] + S > j
            ) <= 1
        )

# Optimize
model.optimize()

# Collect results
results = []
if model.status == GRB.OPTIMAL:
    for i, j, l in x.keys():
        if x[i, j, l].X > 0.5:
            results.append({
                "Object": objects[i],
                "Object_Index": i,
                "Start_Time": j,
                "End_Time": j + Ti[i],
                "Telescope": l
            })

# Create DataFrame and sort
result_df = pd.DataFrame(results)
result_df = result_df.sort_values(by="Start_Time").reset_index(drop=True)

# Add summary row
summary_row = pd.DataFrame([{
    "Object": f"OptimalValue={model.ObjVal:.2f}",
    "Object_Index": f"TotalObserved={len(result_df)}",
    "Start_Time": "",
    "End_Time": f"Cool_Down_S={S}",
    "Telescope": ""
}])

# Append and save
result_df = pd.concat([result_df, summary_row], ignore_index=True)

result_df.to_csv("sol/single_telescope/diff_obstime_and_reward.csv", index=False)
print("Result saved")

print(result_df)

