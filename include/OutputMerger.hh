#ifndef OUTPUTMERGER_HH
#define OUTPUTMERGER_HH

#include <TClass.h>
#include <TFile.h>
#include <TFileMerger.h>
#include <TKey.h>
#include <TString.h>
#include <TSystem.h>
#include <TTree.h>

#include <ctime>
#include <iostream>
#include <map>
#include <set>
#include <vector>

// End-of-run merge for multithreaded Geant4 runs. Each worker thread writes
// <stem>_t<i>.root next to the master's <stem>.root; MergeThreadOutputs
// combines them all into <stem>.root and deletes the per-thread files. Call
// MarkRunStart() from the master's BeginOfRunAction, and MergeThreadOutputs()
// from the master's EndOfRunAction after CloseFile(): Geant4 runs the master's
// EndOfRunAction only after every worker has closed its file.
namespace OutputMerger {

// Geant4 creates an output file only once there is something to write, so a
// thread (or the master) that recorded nothing leaves no file, and a file
// with the same name may be left over from an earlier run in this directory.
// Only files modified since the run started belong to it.
inline Long_t gRunStart = 0;

inline void MarkRunStart() { gRunStart = std::time(nullptr); }

// One second of slack covers the coarse filesystem timestamp.
inline Bool_t WrittenThisRun(const TString &path) {
  FileStat_t stat;
  if (gSystem->GetPathInfo(path, stat) != 0)
    return kFALSE;
  return stat.fMtime >= gRunStart - 1;
}

// Adds the entry count of each top-level TTree in the file to entriesByTree.
inline Bool_t CountTreeEntries(const TString &path,
                               std::map<TString, Long64_t> &entriesByTree) {
  TFile *file = TFile::Open(path, "READ");
  if (!file || file->IsZombie()) {
    delete file;
    return kFALSE;
  }
  // A tree written more than once has a key per cycle, and Get() returns the
  // newest, so each name is counted once.
  std::set<TString> counted;
  TIter nextKey(file->GetListOfKeys());
  TKey *key = static_cast<TKey *>(nextKey());
  while (key) {
    TString name = key->GetName();
    TClass *keyClass = TClass::GetClass(key->GetClassName());
    if (keyClass && keyClass->InheritsFrom(TTree::Class()) &&
        counted.count(name) == 0) {
      counted.insert(name);
      TTree *tree = dynamic_cast<TTree *>(file->Get(name));
      if (tree)
        entriesByTree[name] += tree->GetEntries();
    }
    key = static_cast<TKey *>(nextKey());
  }
  file->Close();
  delete file;
  return kTRUE;
}

// Returns kTRUE once <stem>.root holds every entry this run wrote and its
// per-thread files are deleted. On any failure the per-thread files are kept
// as they are, so a long run can still be merged by hand with hadd. Files
// left over from earlier runs are never merged or deleted.
inline Bool_t MergeThreadOutputs(const TString &stem, Int_t nThreads) {
  TString mergedPath = stem + ".root";
  TString temporaryPath = mergedPath + ".tmp";

  // AccessPathName returns kTRUE when the path does NOT exist.
  std::vector<TString> threadPaths;
  for (Int_t thread = 0; thread < nThreads; thread++) {
    TString threadPath = TString::Format("%s_t%d.root", stem.Data(), thread);
    if (WrittenThisRun(threadPath))
      threadPaths.push_back(threadPath);
    else if (!gSystem->AccessPathName(threadPath))
      std::cerr << "OutputMerger WARNING: ignoring " << threadPath
                << ", left over from an earlier run." << std::endl;
  }
  Int_t nThreadFiles = threadPaths.size();
  if (nThreadFiles == 0) {
    std::cerr << "OutputMerger WARNING: no thread wrote output this run";
    if (!gSystem->AccessPathName(mergedPath) && !WrittenThisRun(mergedPath))
      std::cerr << "; " << mergedPath << " is from an earlier run";
    std::cerr << "." << std::endl;
    return kTRUE;
  }

  std::vector<TString> inputPaths;
  if (WrittenThisRun(mergedPath))
    inputPaths.push_back(mergedPath);
  for (Int_t i = 0; i < nThreadFiles; i++) {
    inputPaths.push_back(threadPaths[i]);
  }

  Int_t nInputs = inputPaths.size();
  std::map<TString, Long64_t> expectedEntries;
  for (Int_t i = 0; i < nInputs; i++) {
    if (!CountTreeEntries(inputPaths[i], expectedEntries)) {
      std::cerr << "OutputMerger ERROR: cannot read " << inputPaths[i]
                << "; per-thread files left unmerged." << std::endl;
      return kFALSE;
    }
  }

  if (!gSystem->AccessPathName(temporaryPath)) {
    std::cerr << "OutputMerger ERROR: refusing to overwrite the stale "
              << temporaryPath << "; per-thread files left unmerged."
              << std::endl;
    return kFALSE;
  }

  Bool_t mergeOk = kFALSE;
  {
    TFileMerger merger(kFALSE);
    merger.SetPrintLevel(0);
    if (merger.OutputFile(temporaryPath.Data(), "RECREATE")) {
      mergeOk = kTRUE;
      for (Int_t i = 0; mergeOk && i < nInputs; i++) {
        mergeOk = merger.AddFile(inputPaths[i].Data(), kFALSE);
      }
      if (mergeOk)
        mergeOk = merger.Merge();
      merger.CloseOutputFile();
    }
  }

  std::map<TString, Long64_t> mergedEntries;
  if (mergeOk)
    mergeOk = CountTreeEntries(temporaryPath, mergedEntries) &&
              mergedEntries == expectedEntries;
  if (!mergeOk) {
    std::cerr << "OutputMerger ERROR: merging into " << mergedPath
              << " failed; per-thread files left unmerged." << std::endl;
    gSystem->Unlink(temporaryPath);
    return kFALSE;
  }

  // rename() replaces the master's file atomically.
  if (gSystem->Rename(temporaryPath, mergedPath) != 0) {
    std::cerr << "OutputMerger ERROR: cannot rename " << temporaryPath
              << " to " << mergedPath << "; per-thread files left unmerged."
              << std::endl;
    return kFALSE;
  }
  for (Int_t i = 0; i < nThreadFiles; i++) {
    gSystem->Unlink(threadPaths[i]);
  }

  std::cout << "Merged " << nThreadFiles << " per-thread files into "
            << mergedPath << ":" << std::endl;
  std::map<TString, Long64_t>::const_iterator tree = mergedEntries.begin();
  while (tree != mergedEntries.end()) {
    std::cout << "  " << tree->first << ": " << tree->second << " entries"
              << std::endl;
    ++tree;
  }
  return kTRUE;
}

} // namespace OutputMerger

#endif
