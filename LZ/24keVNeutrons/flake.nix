{
  description = "Geant4 environment with ROOT.";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.11";
    utils.url = "github:ewtodd/Analysis-Utilities";
  };

  outputs =
    {
      self,
      nixpkgs,
      utils,
    }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
      geant4custom = pkgs.geant4.override { enableQt = true; };
      analysis-utils = utils.packages.${system}.default;

      # Set to true to patch Cf-252 decay data to spontaneous fission only
      patchCf252 = true;

      g4RadioactiveDecay =
        if patchCf252 then
          pkgs.geant4.data.G4RadioactiveDecay.overrideAttrs (old: {
            installPhase = old.installPhase + ''
              cat > $datadir/z98.a252 <<'DECAY'
# 252CF ( 2.645 Y   )
#  Excitation  flag   Halflife  Mode    Daughter Ex flag   Intensity          Q
P            0  - 8.346985e+07
                              SpFission            0               1
                              SpFission            0  -          100        1e-06
DECAY
            '';
          })
        else
          pkgs.geant4.data.G4RadioactiveDecay;
    in
    {
      devShells.${system}.default = pkgs.mkShell {
        buildInputs = with pkgs; [
          root
          geant4custom
          cmake
          analysis-utils
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
          g4RadioactiveDecay
        ];
        shellHook = ''
          export SHELL="/run/current-system/sw/bin/bash"
          export QT_QPA_PLATFORM=wayland
          export QT_WAYLAND_DISABLE_WINDOWDECORATION=1
          export G4VIS_DEFAULT_DRIVER=TSG_QT_ZB

          export AMD_VULKAN_ICD=RADV
          export RADV_PERFTEST=gpl
          export MESA_LOADER_DRIVER_OVERRIDE=radeonsi

          export mesa_glthread=false
          export __GL_THREADED_OPTIMIZATIONS=0

          export DISPLAY=:0
          echo "ROOT version: $(root-config --version)"
          echo "Nuclear-Measurement-Utilities: ${analysis-utils}"

          STDLIB_PATH="${pkgs.stdenv.cc.cc}/include/c++/${pkgs.stdenv.cc.cc.version}"
          STDLIB_MACHINE_PATH="$STDLIB_PATH/x86_64-unknown-linux-gnu"

          ROOT_INC="$(root-config --incdir)"
          # Local first, then remote, then others
          export CPLUS_INCLUDE_PATH="$PWD/include:$STDLIB_PATH:$STDLIB_MACHINE_PATH:${analysis-utils}/include:$ROOT_INC''${CPLUS_INCLUDE_PATH:+:$CPLUS_INCLUDE_PATH}"

          export PKG_CONFIG_PATH="${analysis-utils}/lib/pkgconfig:$PKG_CONFIG_PATH"

          export ROOT_INCLUDE_PATH="$PWD/include:${analysis-utils}/include''${ROOT_INCLUDE_PATH:+:$ROOT_INCLUDE_PATH}"
          # Local lib first means linker will use it preferentially
          export LD_LIBRARY_PATH="$PWD/lib:${analysis-utils}/lib''${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

        '';
      };
    };
}
