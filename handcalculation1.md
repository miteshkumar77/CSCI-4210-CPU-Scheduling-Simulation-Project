| PID | CPU burst time | Arrival time |
| A   | 1              | 3            |
| B   | 12             | 1            |
| C   | 5              | 0            |
| D   | 4              | 1            |
| --- | -------------- | ------------ |

tslice = 3 ms
tcs = 1 ms

A |  >            sX.                                                                                  
B |>    sXXXs            sXXXs   sXXXXXX.                                                                           
C >sXXXs             sXX.                                                                                 
D |>         sXXXs            sX.                                                                         
  +------------------------------------------------------------------------------------------> Time
            1111111111222222222233333333334
  01234567890123456789012345678901234567890

RR

time 0ms: Simulation started [Q empty]
time 0ms: Process C arrived [Q C]
time 1ms: Process B arrived [Q B]
time 1ms: Process D arrived [Q B D]
time 2ms: Process C started using CPU [Q B D]
time 3ms: Process A arrived [Q B D A]
time 4ms: Process C preempted [Q B D A]
time 7ms: Process B started using CPU [Q D A C]
time 9ms: Process B preempted [Q D A C]
time 12ms: Process D started using CPU [Q A C B]
time 14ms: Process D preempted [Q A C B]
time 17ms: Process A started using CPU [Q C B D]
time 18ms: Process A terminated [Q C B D]
time 20ms: Process C started using CPU [Q B D]
time 22ms: Process C terminated [Q B D]
time 24ms: Process B started using CPU [Q D]
time 26ms: Process B preempted [Q D]
time 28ms: Process D began using CPU [Q B]
time 29ms: Process D terminated [Q B]
time 32ms: Process B began using CPU [Q empty]
time 37ms: Process D terminated [Q empty]
time 38ms: Simulator ended for RR [Q empty]