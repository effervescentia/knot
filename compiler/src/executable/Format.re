/**
 Utilities for the "format" command.
 */
open Kore;

type config_t = unit;

let mode = () => {
  let resolve = () => ();

  ("format", [], resolve);
};

let run = (cfg: Compiler.config_t, cmd: config_t) => {
  ();
};
