open Kore;

type config_t = {fix: bool};

let fix_opt = (~default=false, ()) => {
  let value = ref(default);
  let opt =
    Opt.create(
      ~alias="f",
      ~default=Opt.Default.Bool(default),
      "fix",
      Arg.Set(value),
      "automatically apply fixes",
    );
  let resolve = () => value^;

  (opt, resolve);
};

let mode = () => {
  let (fix_opt, get_fix) = fix_opt();

  let resolve = () => {fix: get_fix()};

  ("lint", [fix_opt], resolve);
};

let run = (cfg: Compiler.config_t, cmd: config_t) => {
  ();
};
