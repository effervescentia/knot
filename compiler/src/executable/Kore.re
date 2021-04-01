include Knot.Kore;
include Reference;

module Target = Generate.Target;
module Compiler = Compile.Compiler;

type static_t = {
  root_dir: string,
  source_dir: string,
  out_dir: string,
  target: option(Target.t),
  entry: string,
  debug: bool,
  fix: bool,
  port: int,
};

let defaults = {
  root_dir: Sys.getcwd(),
  source_dir: "src",
  out_dir: "build",
  entry: "main.kn",
  target: None,
  debug: false,
  fix: false,
  port: 1337,
};

type global_t = {
  debug: bool,
  name: string,
  root_dir: string,
  source_dir: string,
};

let panic = (err: string) => {
  Log.fatal("%s", err);

  exit(2);
};
