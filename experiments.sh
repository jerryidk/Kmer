# this script describes a list of experiments. 

invoke()
{
    t=$1
    n=$2
    a=$3
    echo "running test program - ht-size: $t    data-size: $n   alpha: $a "
    ./build/test -t $t -n $n -a $a
}

# Varys alpha value from 0.1 to 0.9
alpha_experiments() 
{
    T=100
    N=100
    a=1.0
    e=10.0
    s=1.0
    while (( $(echo "$a < $e" | bc -l) )); do
        invoke $T $N $a
        a=$(echo "$a + $s" | bc -l)
    done
}

# Vary parititon size
partition_experiments() 
{
    a=0.7
    for ((i = 8; i <= 1024; i *= 2)); do
        invoke $i $i $a
    done   
}


# Vary ht size
ht_experiments() 
{
    n=1024
    a=0.7
    for ((i = $n; i <= 262144; i *= 2)); do
        invoke $i $n $a
    done   
}

make test

if [ "$1" == "alpha" ]; then 
    alpha_experiments 
fi

if [ "$1" == "partition" ]; then 
    partition_experiments
fi

if [ "$1" == "ht" ]; then 
    ht_experiments
fi

