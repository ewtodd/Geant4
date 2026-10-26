{
  description = "Geant4 simulations: one dev shell per project";
  inputs = {
    utils.url = "github:ewtodd/Analysis-Utilities";
    nixpkgs.follows = "utils/nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
  };
  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
      utils,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        au = utils.packages.${system};

        # Geant4 comes from the same nixpkgs Analysis-Utilities pins
        # (nixos-unstable), so it moves only when utils is bumped. Qt is not
        # in the default build, so this one is compiled locally once.
        geant4 = pkgs.geant4.override { enableQt = true; };
        # Each dataset's setup hook exports its G4*DATA variable.
        geant4Data = builtins.filter pkgs.lib.isDerivation (builtins.attrValues geant4.data);

        tools = with pkgs; [
          cmake
          gnumake
          pkg-config
          clang-tools
        ];

        # Qt visualization on the Wayland desktop, rendered by the AMD iGPU.
        visHook = pkgs.lib.optionalString pkgs.stdenv.isLinux ''
          export QT_QPA_PLATFORM=wayland
          export QT_WAYLAND_DISABLE_WINDOWDECORATION=1
          export G4VIS_DEFAULT_DRIVER=TSG_QT_ZB
          export AMD_VULKAN_ICD=RADV
          export RADV_PERFTEST=gpl
          export MESA_LOADER_DRIVER_OVERRIDE=radeonsi
          export mesa_glthread=false
          export __GL_THREADED_OPTIMIZATIONS=0
          export DISPLAY=:0
        '';

        # Every shell: locate the checkout, point the compilers at the
        # project's include/ and lib/, keep the ACLiC cleanup alias.
        projectHook = project: ''
          repo_root="$(git rev-parse --show-toplevel 2>/dev/null || echo "$PWD")"
          project_root="$repo_root/${project}"
          export CPLUS_INCLUDE_PATH="$project_root/include''${CPLUS_INCLUDE_PATH:+:$CPLUS_INCLUDE_PATH}"
          export ROOT_INCLUDE_PATH="$project_root/include''${ROOT_INCLUDE_PATH:+:$ROOT_INCLUDE_PATH}"
          export LD_LIBRARY_PATH="$project_root/lib''${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
          alias clean-aclic='rm -f *_C.so *_C.d *_C_ACLiC_dict_rdict.pcm *_cpp.so *_cpp.d *_cpp_ACLiC_dict_rdict.pcm *_cxx.so *_cxx.d *_cxx_ACLiC_dict_rdict.pcm AutoDict_*'
        '';

        geant4Shell =
          {
            # Path of the project under the repo root.
            project,
            extraInputs ? [ ],
            extraHook ? "",
          }:
          pkgs.mkShell {
            nativeBuildInputs = tools;
            buildInputs = [
              geant4
              au.default
              au.root
              pkgs.bash
            ]
            ++ geant4Data
            ++ extraInputs;
            shellHook = ''
              echo "${project}: Geant4 ${geant4.version}, Analysis-Utilities ${au.default.version}"
              ${projectHook project}
              ${visHook}
              # clangd reads build/compile_commands.json.
              export CMAKE_EXPORT_COMPILE_COMMANDS=ON
              ${extraHook}
              cd "$project_root"
            '';
          };
      in
      {
        devShells = {
          SiDiode = geant4Shell { project = "ANSG/SiDiode"; };
          "Post-OSU-Escape-Peaks" = geant4Shell { project = "ANSG/Post-OSU-Escape-Peaks"; };
          "Post-OSU-Neutron-Activation" = geant4Shell { project = "ANSG/Post-OSU-Neutron-Activation"; };
          DTModeratorOptimization = geant4Shell { project = "ANSG/DTModeratorOptimization"; };
          CZT = geant4Shell { project = "ANSG/Prelim/CZT"; };
          CdTe = geant4Shell { project = "ANSG/Prelim/CdTe"; };
          SiLi = geant4Shell { project = "ANSG/Prelim/SiLi"; };
          GeometryOptimizationHPGe = geant4Shell { project = "ANSG/Prelim/GeometryOptimizationHPGe"; };
          # Two applications, czt96/ and cdte96/, each built in its own build/.
          "73mGe-Escape96" = geant4Shell { project = "ANSG/73mGe-Escape96"; };

          Cf252ModeratorOptimization = geant4Shell { project = "MDvDM/Cf252ModeratorOptimization"; };
          LaBr3TaggingEfficiency = geant4Shell { project = "MDvDM/LiF/LaBr3TaggingEfficiency"; };
          ModeratedCf252LaBr3CeBr3Coincidence = geant4Shell {
            project = "MDvDM/LiF/ModeratedCf252LaBr3CeBr3Coincidence";
          };

          "Si-dE" = geant4Shell { project = "MUSIC/Si-dE"; };
          Coincidence = geant4Shell { project = "PSI/Coincidence"; };

          default = pkgs.mkShell {
            shellHook = ''
              echo "Pick a project: nix develop .#<Project>"
              echo "  ANSG:  SiDiode | Post-OSU-Escape-Peaks | Post-OSU-Neutron-Activation | DTModeratorOptimization"
              echo "         CZT | CdTe | SiLi | GeometryOptimizationHPGe   (ANSG/Prelim)"
              echo "         73mGe-Escape96"
              echo "  MDvDM: Cf252ModeratorOptimization | LaBr3TaggingEfficiency | ModeratedCf252LaBr3CeBr3Coincidence"
              echo "  MUSIC: Si-dE"
              echo "  PSI:   Coincidence"
            '';
          };
        };
      }
    );
}
