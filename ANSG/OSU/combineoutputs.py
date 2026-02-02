import ROOT
import os
import sys


def combine_root_files(output_file, input_files):
    CdTe_chain = ROOT.TChain("CdTe")

    # Add files to the chains
    for file_name in input_files:
        CdTe_chain.Add(file_name)
    # Create the output file
    output = ROOT.TFile(output_file, "RECREATE")

    # Merge the chains into the output file
    output.cd()
    CdTe_tree = CdTe_chain.CloneTree(-1, "fast")
    CdTe_tree.Write()
    # Close the output file
    output.Close()
    print(f"Combined {len(input_files)} ROOT files into {output_file}")


# List of input ROOT files (one for each thread)
input_files = [f"build/output0_t{i}.root" for i in range(12)]

# Output ROOT file
if len(sys.argv) != 1:
    output_file = sys.argv[1] + ".root"
else:
    output_file = "combined_output.root"

# Combine the files
combine_root_files(output_file, input_files)
