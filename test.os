def func2(i):
	j = i * 10
	print(j)

def func1(i):
	j = i * 3
	print(j)
	func2(j)

i = ( 5 + 10 ) * (10+15*6)
print(i)
func1(i)
