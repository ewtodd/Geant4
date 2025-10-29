import ROOT
import os
import sys


def combine_root_files(output_file, input_files):
    # Create chains for both detector ntuples
    czt_chain = ROOT.TChain("CZT")
    hpge_chain = ROOT.TChain("HPGe")

    # Add files to both chains, checking if they exist and are valid
    valid_files = []
    for file_name in input_files:
        if os.path.exists(file_name):
            # Check if file is valid ROOT file and contains expected trees
            test_file = ROOT.TFile(file_name, "READ")
            if test_file and not test_file.IsZombie():
                czt_tree = test_file.Get("CZT")
                hpge_tree = test_file.Get("HPGe")
                if czt_tree and hpge_tree:
                    czt_chain.Add(file_name)
                    hpge_chain.Add(file_name)
                    valid_files.append(file_name)
                else:
                    print(
                        f"Warning: File {file_name} missing CZT or HPGe trees, skipping..."
                    )
            else:
                print(f"Warning: File {file_name} is corrupted, skipping...")
            test_file.Close()
        else:
            print(f"Warning: File {file_name} not found, skipping...")

    if len(valid_files) == 0:
        print("Error: No valid files found!")
        return

    # Create the output file
    output = ROOT.TFile(output_file, "RECREATE")
    output.cd()

    # Clone both trees into the output file - check for null pointers
    czt_entries = czt_chain.GetEntries()
    hpge_entries = hpge_chain.GetEntries()

    print(f"CZT chain entries: {czt_entries}")
    print(f"HPGe chain entries: {hpge_entries}")

    if czt_entries > 0:
        czt_tree = czt_chain.CloneTree(-1, "fast")
        if czt_tree:
            czt_tree.Write()
        else:
            print("Error: Failed to clone CZT tree")
    else:
        print("Warning: No CZT entries found")

    if hpge_entries > 0:
        hpge_tree = hpge_chain.CloneTree(-1, "fast")
        if hpge_tree:
            hpge_tree.Write()
        else:
            print("Error: Failed to clone HPGe tree")
    else:
        print("Warning: No HPGe entries found")

    # Close the output file
    output.Close()

    # Print summary information
    print(f"Combined {len(valid_files)} valid ROOT files into {output_file}")


# List of input ROOT files (one for each thread)
input_files = [f"build/output0_t{i}.root" for i in range(24)]

# Output ROOT file
if len(sys.argv) != 1:
    output_file = sys.argv[1] + ".root"
else:
    output_file = "combined_output.root"

# Combine the files
combine_root_files(output_file, input_files)
