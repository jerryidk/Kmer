chmod +x ./experiments.sh
./experiments.sh ht > output_ht.txt
cat output_ht.txt | awk -F ',' '/per_op/ {print $2,$5,$6}'