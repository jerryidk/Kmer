T=100
N=100

invoke()
{
    t=$1
    n=$2
    a=$3
    echo "running test program - ht-size: $t    data-size: $n   alpha: $a "
    ./build/test -t $t -n $n -a $a
}


alpha_experiments() 
{
    
    a=0.1
    e=1.0
    s=0.1
    while (( $(echo "$a < $e" | bc -l) )); do
        invoke $T $N $a
        a=$(echo "$a + $s" | bc -l)
    done
}


make test
alpha_experiments 