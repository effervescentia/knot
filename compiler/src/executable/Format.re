/**
 Utilities for the "format" command.
 */
open Kore;

type config_t = unit;

let mode = () => {
  let resolve = () => ();

  ("format", [], resolve);
};

let run = (~catch=print_errs % panic, cfg: Compiler.config_t, cmd: config_t) => {
  let compiler =
    Compiler.create(
      ~catch=
        List.filter(
          fun
          | ImportCycle(_)
          | UnresolvedModule(_)
          | FileNotFound(_) => false,
        )
        % (errors => errors |> List.is_empty ? () : catch(errors)),
      cfg,
    );

  compiler |> Compiler.init(~skip_cache=true);
  compiler
  |> Compiler.emit_output(
       Target.Knot,
       Filename.concat(cfg.root_dir, cfg.source_dir),
     );
  compiler |> Compiler.teardown;
};
