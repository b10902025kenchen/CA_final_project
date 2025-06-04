import os
import random
import sys
import numpy as np
def generate_para_file(machine_num, switch_val,intervals):
    para_file = "testcase_para.txt"
    with open(para_file, 'w') as f:
        f.write(f"MACHINES {machine_num}\n")
        f.write(f"INTERVALS {intervals}\n")
        f.write(f"SWITCH_TIME {switch_val}\n")
    print(f"Parameter file '{para_file}' generated.")

def generate_score_file(star_num):
    score_file = "testcase_score.txt"
    with open(score_file, 'w') as f:
        for i in range(star_num):
            score = random.randint(1, 10)
            f.write(f"{score}\n")
    print(f"Score file '{score_file}' generated.")

def generate_observation_file(star_num, intervals):
    observation_file = "testcase_observation.txt"
    with open(observation_file, 'w') as f:
        for i in range(star_num):   #int from 1 to intervals*0.1
            observation = random.randint(1, int(intervals * 0.1))
            f.write(f"{observation}\n")
    print(f"Observation file '{observation_file}' generated.")

def generate_star_file(star_num,intervals):
    star_file = "testcase_star.csv"
    with open(star_file, 'w') as f:
        for i in range(star_num):
            start_time = 0
            end_time = intervals
            moon_start = 500
            moon_end = 500
            star = f"M{i},{start_time},{end_time},{moon_start},{moon_end}"
            f.write(f"{star}\n")
    print(f"Star file '{star_file}' generated.")

def main():
    if len(sys.argv) != 5:
        print("Usage: python testcase_gen.py <machine_num> <switch_val> <intervals> <star_num>")
        sys.exit(1)

    machine_num = int(sys.argv[1])
    switch_val = int(sys.argv[2])
    intervals = int(sys.argv[3])
    star_num = int(sys.argv[4])

    generate_para_file(machine_num, switch_val, intervals)
    generate_score_file(star_num)
    generate_observation_file(star_num, intervals)
    generate_star_file(star_num, intervals)

if __name__ == "__main__":
    main()