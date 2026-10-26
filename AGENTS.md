# Mandatory Rules
<!---->
## In all languages
<!---->
- Prefer slightly verbose, self-explanatory code over terse code that needs
  comments to be understood.
- Keep comments to only what explains something non-obvious.
- Never embed a literal `\n` inside a string or print argument.
  A line break is always its own explicit statement.
  In C++/ROOT, use `std::cout << ... << std::endl;` (`G4cout << ... <<
  G4endl;` in Geant4 code).
  In Python, split output into separate `print()` calls, and use a bare
  `print()` for a blank line rather than appending `\n`.
<!---->
## Project layout
<!---->
- This is an umbrella repo of Geant4 simulations: one flake, one dev shell
  per project, named after the project's directory.
  - `ANSG/`: `SiDiode`, `Post-OSU-Escape-Peaks`,
    `Post-OSU-Neutron-Activation`, `DTModeratorOptimization`,
    `73mGe-Escape96`, and under `ANSG/Prelim/`: `CZT`, `CdTe`, `SiLi`,
    `GeometryOptimizationHPGe`.
  - `MDvDM/`: `Cf252ModeratorOptimization`, and under `MDvDM/LiF/`:
    `LaBr3TaggingEfficiency`, `ModeratedCf252LaBr3CeBr3Coincidence`.
  - `MUSIC/`: `Si-dE`. `PSI/`: `Coincidence`.
- Run `nix develop .#<Project>` from the repo root. Never `nix-shell`. The
  shell starts in the project directory, and wires that project's
  `include/` into `CPLUS_INCLUDE_PATH`/`ROOT_INCLUDE_PATH` and `lib/` into
  `LD_LIBRARY_PATH`. Every shell has the same Geant4 (with Qt and all its
  datasets, `G4*DATA` set), Analysis-Utilities' ROOT, and Analysis-Utilities.
- A project is one Geant4 application: `*.cc`/`*.hh`, `CMakeLists.txt`, and
  its macros. Build it from the project directory with
  `cmake -S . -B build && make -C build -j`, and run `build/sim <macro>`.
  `73mGe-Escape96` holds two applications, `czt96/` and `cdte96/`, each
  built in its own `build/`; `run_escape96.sh` builds and runs them.
- Every sim merges its per-thread outputs at the end of a run: the master's
  `EndOfRunAction` calls `OutputMerger::MergeThreadOutputs` from
  `include/OutputMerger.hh`, which merges `output<run>_t<i>.root` into
  `output<run>.root` and deletes the per-thread files once the merged file
  is verified. A new sim must do the same: `OutputMerger::MarkRunStart()`
  in the master's `BeginOfRunAction`, `MergeThreadOutputs` after
  `CloseFile()` in its `EndOfRunAction`, and in `CMakeLists.txt`
  `find_package(ROOT)`, the repo's `include/` on the include path, and
  `${ROOT_LIBRARIES}` linked.
- Analysis is ROOT C++ on Analysis-Utilities, in the project's `macros/`,
  run from there with `root -l Macro.cpp+`. Macros read
  `root_files/<name>.root` and save figures with
  `PlottingUtils::SaveFigure` into `macros/plots/`. Code shared by several
  macros goes in the project's `include/` and `src/`, built into `lib/` by
  a `Makefile` after Analysis-Utilities' template (`make` in the project
  directory; `CZT` does this).
- `Cf252ModeratorOptimization` names its output after the moderator
  thickness, `Poly<t>cm.root`, not `output<run>.root`; `/moderator/thickness`
  changes it between runs, and `scan.mac` runs 5 to 12 cm.
- `GeometryOptimizationHPGe` takes the crystal thickness and side length
  (cm) before the macro: `build/sim <thickness> <length> run.mac`.
- Some generators read input files from the working directory: `CZT`,
  `CdTe` and `SiLi` read `optimized.root`, and `GeometryOptimizationHPGe`
  reads `thermalEnergySpectra.root`, which `DTModeratorOptimization`'s
  `AnalyzeAndPlotSpectra.cpp` writes. Both are tracked in
  `ANSG/Prelim/inputs/` (the one exception to `*.root` in `.gitignore`),
  and each project's CMake copies the one it reads into `build/`.
<!---->
## Data
<!---->
- Simulation outputs never go in git (`*.root` is ignored). They live on
  `/labdata/<group>/Geant4/<project path below the group>/`, for example
  `/labdata/ANSG/Geant4/Prelim/CZT/`, and each project reaches them through
  the gitignored symlink `macros/root_files`. Move a finished run's
  `output<run>.root` there under a descriptive name.
- `73mGe-Escape96` writes its runs straight into UM-ANSG's
  `73mGe/root_files/geant4/<geometry>_<run>/`.
- Never delete simulation outputs without asking.
<!---->
## Nix
<!---->
- Always use flakes and flake-based commands (`nix run`, `nix shell`, etc.).
  Never use the old `nix-shell` approach.
- If you are confused, stop and ask for help.
  This is especially critical in Nix.
- Follow the existing style of the surrounding modules.
- `utils` (Analysis-Utilities) is a pinned flake input, and `nixpkgs`
  follows `utils/nixpkgs` (nixos-unstable), so Geant4 and its datasets move
  only when `utils` does. Bump it with `nix flake lock --update-input utils`,
  never by hand-editing the lock file.
- ROOT is always Analysis-Utilities' (`au.root`), never `pkgs.root`: the
  library is built against it, and a different ROOT is a different ABI.
<!---->
## C++ / ROOT
<!---->
- Use ROOT data types, and pick the *correct* one for the actual need rather
  than defaulting blindly: `Int_t` for ordinary ints, `Long64_t` for entry
  counts and large/64-bit values, `Double_t` for floating point, `TString`
  for string convenience, and so on.
  Match the width and signedness the code actually requires.
  Geant4 classes keep Geant4's own types (`G4int`, `G4double`, `G4String`)
  and units (`G4SystemOfUnits`).
- Do not use modern C++ features: no `auto`, no smart pointers, no
  range-based (`for (x : c)`) iteration.
  Use explicit types and classic indexed/iterator loops.
- Lambdas are permitted where they are short, local, and improve readability
  over the alternatives: sort comparators defined next to the std::sort call,
  thread workers whose explicit parameter lists would be longer and harder to
  scan than an inline capture.
  A lambda that spans more than about 5 lines or captures by reference
  outside an immediately obvious scope (e.g. stored in a std::function
  returned from the function) should still be a named function.
  When in doubt, write a named function.
- In performance-critical code (stepping and event actions above all),
  always gate logging behind a compile- or run-time toggle so it can be
  disabled.
  The `std::endl` flush is therefore never a concern on hot paths.
<!---->
## Python
<!---->
- In Python that uses ROOT, never use matplotlib.
  Look at nearby files for the established plotting approach, or ask which
  is preferred.
<!---->
## Explanations
<!---->
- For non-trivial changes, explain thoroughly what changed and why.
  Do not over-summarize or truncate the reasoning.
  Trivial edits can stay terse.
<!---->
## Commits
<!---->
- A Conventional Commits summary line with scope (`feat(SiDiode): …`,
  `fix(ANSG): …`, `build(nix): …`, `docs: …`), a blank line, then a body
  that explains what changed and why.
- Agent commits: `--author="son-of-anton-bot <307402699+son-of-anton-bot@users.noreply.github.com>"`.
- Stage explicit paths; never `git add .` or `git add -A`.
- A `pre-commit` hook in `.githooks/` clang-formats staged C/C++ (including
  Geant4's `.cc`/`.hh`) with `LLVM, BreakStringLiterals: false` and
  re-stages the result.
  `BreakStringLiterals` is deliberately off, so do not reformat string
  literals by hand.
  Git finds the hook through `core.hooksPath = .githooks`; set it with
  `git config core.hooksPath .githooks` if your git config does not.
