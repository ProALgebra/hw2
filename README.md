Отчёт о прохождении тестов

Все тесты в regression исключая 9000 проходимы интерпретатором байткода. 9000 тест не работает в силу особенностей компилятора
Можно проверить через make regression
```
LAMA=../runtime ../src/lamac -b test9000.lama && cat test9000.input | ../byterun/interpreter test9000.bc > test9000.log && diff test9000.log orig/test9000.log
Fatal error: exception Failure("ERROR: undefined label 'Lstringcat'")
```
Так же тесты производительности
```

Sort
`which time` -f "Sort\t%U" lamac -i Sort.lama </dev/null
Sort    10.24
make[1]: Leaving directory '/home/doctor/VM/hw2/performance'
lamac -b ../performance/Sort.lama
`which time` -f "Sort time\t%U" ./interpreter Sort.bc
Sort time       6.10
```