/**
 Utilities for writing to disk.
 */
open Kore;

let write = (out: out_channel, pretty: Pretty.t) =>
  Pretty.flatten(Printf.fprintf(out, "%s"), 0, [(pretty, 0)]);
