import ROOT
import os
import sys


def combine_root_files(output_file, input_files):
    chain = ROOT.TChain("CZT")

    for file_name in input_files:
        chain.Add(file_name)
    output = ROOT.TFile(output_file, "RECREATE")

    output.cd()
    tree = chain.CloneTree(-1, "fast")
    tree.Write()
    output.Close()
    print(f"Combined {len(input_files)} ROOT files into {output_file}")


input_files = [f"build/output0_t{i}.root" for i in range(32)]

if len(sys.argv) != 1:
    output_file = sys.argv[1] + ".root"
else:
    output_file = "combined_output.root"

combine_root_files(output_file, input_files)
