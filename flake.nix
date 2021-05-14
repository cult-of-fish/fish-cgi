{
  inputs.nixpkgs.url = "nixpkgs/nixpkgs-unstable";
  outputs = { self, nixpkgs, ... }:
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
        src = pkgs.nix-gitignore.gitignoreSource [
          "flake.nix"
        ] ./.;
        nativeBuildInputs = [ pkgs.bash pkgs.cmake ];
        buildInputs = [ fish pkgs.fcgi ] ++ fish.deps;

        cmakeFlags = [ "-DFETCH_FISH=OFF" "-DFISH_PATH=${fish}" ];
        postInstall = "mkdir -p $out/share; cp $src/examples/index.fish $out/share";
      };

      devShell.${sys} = self.defaultPackage.${sys}.overrideAttrs (o: {
        nativeBuildInputs = o.nativeBuildInputs ++ [ pkgs.clang-tools ];
      });

      # https://github.com/NixOS/nixpkgs/blob/nixos-20.09/nixos/modules/services/web-servers/phpfpm/default.nix
      # TODO(tny): parameterize
      nixosModule = { config, lib, ... }: with lib; let cfg = config.services.fishcgi; in {
        options = {
          services.fishcgi.enable = mkOption {
            type = types.bool;
            default = false;
          };
          services.fishcgi.package = mkOption {
            type = types.package;
            default = self.defaultPackage.${sys};
          };
          services.fishcgi.example = mkOption {
            type = types.str;
            default = "${self.defaultPackage.${sys}}/share/index.fish";
            readOnly = true;
          };

          services.fishcgi.socket = mkOption {
            type = types.str;
            default = "/run/fishcgi.sock";
            readOnly = true;
          };
        };
        config = lib.mkIf cfg.enable {
          users.users.fishcgi.isSystemUser = true;
          systemd.sockets.fishcgi = {
            wantedBy = [ "sockets.target" ];
            socketConfig = {
              SocketUser = "nginx";
              ListenStream = cfg.socket;
              Accept = false;
            };
          };
          systemd.services.fishcgi = {
            after = [ "network.target" ];
            serviceConfig = {
              Type = "simple";
              User = "fishcgi";
              Group = "nginx";
              ExecStart = "${cfg.package}/bin/fish-cgi";
              StandardInput = "socket";
            };
          };
        };
      };
    };
}
