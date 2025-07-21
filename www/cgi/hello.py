from sys import argv

if len(argv) == 2:
    print('hello: ', argv[1], sep='')
else:
    print('usage: "python3 hello.py {{name}}"')