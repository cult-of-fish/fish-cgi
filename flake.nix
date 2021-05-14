{
  inputs.nixpkgs.url = "nixpkgs/nixpkgs-unstable";
  outputs = { nixpkgs, ... }:
    let
      lib = nixpkgs.lib;
      sys = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${sys};
      fish = pkgs.fetchFromGitHub {
        owner = "fish-shell";
        repo = "fish-shell";
        rev = "3.2.2";
        hash = "sha256-oepfqB6UEP48ysLc93pNEe5Hl0UbWIvS6R1kKbbwKO0=";

        # epic hack: remove shebang and a shell interpreter ends up executing the file anyways
        extraPostFetch = "sed -i 1d $out/build_tools/git_version_gen.sh";
        passthru.deps = with pkgs; [ ncurses libiconv pcre2 gettext ];
      };
    in {
      defaultPackage.${sys} = pkgs.stdenv.mkDerivation {
        pname = "fish-cgi";
        version = "HEAD";
        src = ./.;
        nativeBuildInputs = [ pkgs.bash pkgs.cmake ];
        buildInputs = [ fish pkgs.fcgi ] ++ fish.deps;

        cmakeFlags = [ "-DFETCH_FISH=OFF" "-DFISH_PATH=${fish}" ];
      };
    };
}
