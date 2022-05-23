{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  name = "yafetch";

  nativeBuildInputs = with pkgs; [
    clang
    lldb
    cmake
  ];
}
