{
  description = "Geant4 environment with ROOT.";
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.11";
    flake-utils.url = "github:numtide/flake-utils";
    utils.url = "github:ewtodd/Analysis-Utilities";
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
        geant4custom = pkgs.geant4.override { enableQt = true; };
        analysis-utils = utils.packages.${system}.default;
        isDarwin = pkgs.stdenv.isDarwin;
      in
      {
        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            root
            geant4custom
            cmake
            analysis-utils
            zsh
            geant4.data.G4ABLA
            geant4.data.G4INCL
            geant4.data.G4PhotonEvaporation
            geant4.data.G4RealSurface
            geant4.data.G4EMLOW
            geant4.data.G4NDL
            geant4.data.G4PII
            geant4.data.G4SAIDDATA
            geant4.data.G4ENSDFSTATE
            geant4.data.G4PARTICLEXS
            geant4.data.G4TENDL
            geant4.data.G4RadioactiveDecay
            (python3.withPackages (python3Packages: [
              python3Packages.numpy
              python3Packages.root
            ]))
          ];
          shellHook = ''
            export SHELL="${pkgs.zsh}/bin/zsh"
            ${
              if !isDarwin then
                ''
                  export QT_QPA_PLATFORM=wayland
                  export QT_WAYLAND_DISABLE_WINDOWDECORATION=1
                  export G4VIS_DEFAULT_DRIVER=TSG_QT_ZB
                  export AMD_VULKAN_ICD=RADV
                  export RADV_PERFTEST=gpl  
                  export MESA_LOADER_DRIVER_OVERRIDE=radeonsi
                  export mesa_glthread=false
                  export __GL_THREADED_OPTIMIZATIONS=0
                  export DISPLAY=:0
                ''
              else
                ""
            }
            echo "ROOT version: $(root-config --version)"
            echo "Analysis-Utilities: ${analysis-utils}"
            STDLIB_PATH="${pkgs.stdenv.cc.cc}/include/c++/${pkgs.stdenv.cc.cc.version}"
            STDLIB_MACHINE_PATH="$STDLIB_PATH/${
              if isDarwin then "arm64-apple-darwin" else "x86_64-unknown-linux-gnu"
            }"
            ROOT_INC="$(root-config --incdir)"
            # Local first, then remote, then others
            export CPLUS_INCLUDE_PATH="$PWD/include:$STDLIB_PATH:$STDLIB_MACHINE_PATH:${analysis-utils}/include:$ROOT_INC''${CPLUS_INCLUDE_PATH:+:$CPLUS_INCLUDE_PATH}"
            export PKG_CONFIG_PATH="${analysis-utils}/lib/pkgconfig:$PKG_CONFIG_PATH"
            export ROOT_INCLUDE_PATH="$PWD/include:${analysis-utils}/include''${ROOT_INCLUDE_PATH:+:$ROOT_INCLUDE_PATH}"
            # Local lib first means linker will use it preferentially
            export LD_LIBRARY_PATH="$PWD/lib:${analysis-utils}/lib''${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
            exec ${pkgs.zsh}/bin/zsh
          '';
        };
      }
    );
}
