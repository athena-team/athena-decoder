
curl "localhost:5063/offlineasr" -s -X POST -T test.pcm 

#for((i=0;i<10;i++));do
#    curl "localhost:5063/offlineasr" -s -X POST -T test1.pcm &
#done
#wait
