import ROOT
import os
import sys


def combine_root_files(output_file, input_files):
    # Create a TChain for each tree type
    exit_chain = ROOT.TChain("PreEnergy")
    e_dep_chain = ROOT.TChain("LaBr3")
    entries_chain = ROOT.TChain("LaBr3Entries")

    # Add files to the chains
    for file_name in input_files:
        exit_chain.Add(file_name)
        e_dep_chain.Add(file_name)
        entries_chain.Add(file_name)

    # Create the output file
    output = ROOT.TFile(output_file, "RECREATE")

    # Merge the chains into the output file
    output.cd()
    exit_tree = exit_chain.CloneTree(-1, "fast")
    e_dep_tree = e_dep_chain.CloneTree(-1, "fast")
    entries_tree = entries_chain.CloneTree(-1, "fast")

    # Write the trees to the output file
    exit_tree.Write()
    e_dep_tree.Write()
    entries_tree.Write()
    # Close the output file
    output.Close()
    print(f"Combined {len(input_files)} ROOT files into {output_file}")


# List of input ROOT files (one for each thread)
input_files = [f"build/output0_t{i}.root" for i in range(32)]

# Output ROOT file
if len(sys.argv) != 1:
    output_file = sys.argv[1] + ".root"
else:
    output_file = "combined_output.root"

# Combine the files
combine_root_files(output_file, input_files)
