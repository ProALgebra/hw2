```
make[1]: Entering directory '/home/doctor/VM/test/hw2/performance'
Sort
`which time` -f "Sort\t%U" lamac -i Sort.lama </dev/null
Sort    10.72
Sort
../src/lamac  Sort.lama && `which time` -f "Sort\t%U" ./Sort
Sort    2.46
make[1]: Leaving directory '/home/doctor/VM/test/hw2/performance'
lamac -b ../performance/Sort.lama
new interpreter:
`which time` -f "Sort time\t%U" ./interpreter Sort.bc
Sort time       5.72
old interpreter:
`which time` -f "Sort time\t%U" ./interpreter_old Sort.bc
Sort time       6.43
```
Выводы, когда мы убрали проверки мы стали быстрее на 10+%