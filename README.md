I was asked how I would calculate the distributed median in an interview and thought it was a fairly interesting question so I decided to write a program to do it. This is a copy of that program since the repo it is in has many other commits and contains projects that I don't want to push public now.

This calculates the median across however many machines are specified (not a cli option, have to change in code). This only works in the case where the number of numbers is odd. If the number is even, the program will likely never terminate. This is because the algorithm works its way down to the middle element, but no such element exists of there are an even number of numbers.

There are multiple ways to solve this issue. I could make it so that the master program determines if the count is even or odd. If even, it could simply run once looking for the "upper median" value, and again looking for the "lower median" value. This could then be averaged. Of course, this effectively doubles the runtime. Another option would be to "cheat" and call either the lower or upper median the true median. This cuts down on runtime and in cases where there are likely many instances of each number, this is probably the better option.

For now, I've opted to leave it as is. Much of the code here has working comments or had the comments removed since this is just a mockup solution.
