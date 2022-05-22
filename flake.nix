rec {
  description = "yet another fetch, written in C++";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
  };

  outputs = { self, nixpkgs }: let
    version = builtins.substring 0 8 self.lastModifiedDate;

    forAllSystems = nixpkgs.lib.genAttrs [ "x86_64-linux" "aarch64-linux" ];

    # Nixpkgs instantiated for each supported system
    nixpkgsFor = forAllSystems (system:
      import nixpkgs {
        inherit system;
        overlays = [ self.overlays.default ];
      });
  in {
    overlays.default = final: prev: {
      yafetch = with final; clangStdenv.mkDerivation rec {
        pname = "yafetch";
        inherit version;

        src = self;

        nativeBuildInputs = [
          cmake
        ];

        meta = with lib; {
          inherit description;
          homepage = "https://github.com/kira64xyz/yafetch";
          license = licenses.gpl3Plus;
          platforms = platforms.linux;
        };
      };
    };

    packages = forAllSystems (system: {
      inherit (nixpkgsFor.${system}) yafetch;
      default = (nixpkgsFor.${system}).yafetch;
    });
  };
}
