{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  name = "yafetch";

  nativeBuildInputs = with pkgs; [
    clang
    gnumake
    gdb
  ];

}
