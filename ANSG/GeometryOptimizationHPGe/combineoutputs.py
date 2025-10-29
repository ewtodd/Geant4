import ROOT
import os
import sys

def combine_root_files(output_file, input_files):
    # Create a TChain for each tree type
    energy_chain = ROOT.TChain("Energy")
    counts_chain = ROOT.TChain("Counts Produced")
    esc_chain = ROOT.TChain("Counts Escaped")


    # Add files to the chains
    for file_name in input_files:
        energy_chain.Add(file_name)
        counts_chain.Add(file_name)
        esc_chain.Add(file_name)
       
    # Create the output file
    output = ROOT.TFile(output_file, 'RECREATE')

    # Merge the chains into the output file
    output.cd()
    energy_tree = energy_chain.CloneTree(-1, "fast")
    counts_tree = counts_chain.CloneTree(-1, "fast")
    esc_tree = esc_chain.CloneTree(-1, "fast")
        # Write the trees to the output file
    energy_tree.Write()
    counts_tree.Write()
    esc_tree.Write()
    

    # Close the output file
    output.Close()
    print(f"Combined {len(input_files)} ROOT files into {output_file}")

# List of input ROOT files (one for each thread)
input_files = [f'build/output0_t{i}.root' for i in range(16)]

# Output ROOT file
if (len(sys.argv) != 1):
    output_file = sys.argv[1] + '.root'
else: output_file = 'combined_output.root'

# Combine the files
combine_root_files(output_file, input_files)
