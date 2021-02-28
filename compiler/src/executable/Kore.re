include Knot.Kore;
include Resolve.Kore;
include Generate.Kore;

module Compiler = Compile.Compiler;

let panic = (err: string) => {
  Log.fatal("%s", err);

  exit(2);
};
