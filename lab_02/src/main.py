from argparse import ArgumentParser

parser = ArgumentParser()
parser.add_argument('x', type=float, help='x = ')
parser.add_argument('n', type=int, help='n = ')
args = parser.parse_args()

x,n = args.x, args.n

print(f'x = {x}\nn = {n}')

def calculate(x):
    return x**2-x**2+x*4-x*5+x+x

from timeit import repeat
def assign():
    global x
    x = calculate(x)
t = repeat(assign, repeat= 1, number=n)[0]

print(f'x\' = {x}')
print(f'{t*1000} ms')