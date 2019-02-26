#!/bin/sh
COUNT=1000
i=0
max=0
min=10000000
total=0
while [ ${i} -lt ${COUNT} ] 
do
start=`echo $[$(date +%s%N)]`
curl http://127.0.0.1:8080 > /dev/null 2>&1 &
i=`expr ${i} + 1`
end=`echo $[$(date +%s%N)]`

temp=`expr ${end} - ${start}`

if [ ${temp} -ge ${max} ]
then
max=${temp}
fi

if [ ${temp} -le ${min} ]
then
min=${temp}
fi

total=`expr ${total} + ${temp}`
done
echo "总耗时: ${total} 纳秒;"
echo "最大耗时: ${max} 纳秒;"
echo "最小耗时: ${min} 纳秒."