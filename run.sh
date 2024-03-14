# chmod +x ./experiments.sh
# ./experiments.sh ht > output_ht.txt
# cat output_ht.txt | awk -F ',' '/per_op/ {print $2,$5,$6}'


ht_size=400
data_size=100
path=/opt/dramhit/kmer_dataset/SRR1513870.fastq
k=10
num_threads=64
seq_len=190
lc=156097668
./build/kmer -t $ht_size -n $data_size  -s $num_threads -p $path -c $lc  -k $k -s $num_threads -N $seq_len > kmer_out.txt
 