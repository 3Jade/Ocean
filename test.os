function func2(i)
{
	var j = i * 10;
	print(j);
}

function func1(i)
{
	var j = i * 3;
	print(j);
	func2(j);
}

var i = ( 5 + 10 ) * (10+15*6);
print(i);
func1(i);
