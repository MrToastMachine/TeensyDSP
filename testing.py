filename = "sineWave.csv"


with open(filename, 'r') as file:
    arr = file.read().split(',')

    print("num vals:", len(arr))
    for i in range(5):
        print(type(arr[i]),arr[i])
        
    float_arr = [float(i) for i in arr]

    print("num vals:", len(float_arr))
    for i in range(5):
        print(type(float_arr[i]),float_arr[i])


print(f"sum of file: {sum(float_arr)}")
    
