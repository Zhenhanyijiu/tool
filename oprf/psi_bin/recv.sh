date
#     0:send,1:recv  
time ./psi -r 1 -w 128 -h 20 -ss 5000000  -rs 5000000 -sd 20 -ids 21 -omp $1
# time ./psi -r 1 -w 100 -h 20 -ss 100000000  -rs 100000000 -sd 110 -ids 21 -ip 172.16.48.172 -omp $1
# time ./psi -r 1 -w 100 -h 20 -ss 10000000  -rs 10000000 -sd 110 -ids 32 -ip 172.16.48.172 -omp 8
# time ./psi -r 1 -w 128 -h 20 -ss 100000000  -rs 100000000 -sd 210 -ids 21 -ip 172.16.48.172
#./psi -r 1 -w 100 -h 20 -ss 60000000 -rs 3000000 -sd 22 
#./psi -r 1 -w 100 -h 20 -ss 3000000 -rs 60000000 -sd 22 -ip 172.22.47.82 -port 70000
#./psi -r 1 -w 200 -h 16 -ss 30000000 -rs 30000000 -sd 22 -ip 172.16.65.3 -port 70000
#./psi -r 1 -w 60 -h 10 -ss 30000000 -rs 30000000 -sd 22 
#-in ../../../500w_2.txt -out ../../../500w_2_save.txt