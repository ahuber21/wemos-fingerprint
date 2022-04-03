with open("codes.txt") as fp:
    lines = fp.readlines()

for line in lines:
    if "FPM_" not in line:
        continue

    cmd = line.split()[1]
    print(f"    case {cmd}:")
    print(f'        Serial.println("{cmd}";')
    print(f"        break;")


print("\n" * 10)


for line in lines:
    if "FPM_" not in line:
        continue

    cmd = line.split()[1]
    print(f"    case {cmd}:")
    print(f'        return "{cmd}";')
