include Knot.Kore;
include Resolve.Kore;

module Compiler = Compile.Compiler;

type mode_t =
  | Static
  | Server
  | Watch;
