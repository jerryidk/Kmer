
# Kmer counting Project

Using system knowledge to speed up HT insertion for kmer workload.

# TODO

Logistic
- Cloudlab profile setup

Development
- Kmer: Add few common command line parameters
- Util: Add a precise timer to measure precise time of the code. 

Experiment
- Varys equal size of Partition up to 128 MB, and look at the cpo
- Varys alpha from 0.0 - 1.0 in some fixed step, each Partition with equal size Data and HT
- Varys 1X to 10X HT with different size Data, fix alpha 

# Build instruction

Build Kmer counter

`make kmer` 

Build Test or Experiements 

`make test`

# Experiments

Run a script that corresponds to a certain experiments
