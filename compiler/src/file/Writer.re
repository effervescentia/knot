/**
 Utilities for writing to disk.
 */
open Kore;

let write_pretty = (out: out_channel, pretty: Pretty.t) =>
  Pretty.flatten(Printf.fprintf(out, "%s"), 0, [(pretty, 0)]);

let write = (out: out_channel, writer: Format.formatter => unit) =>
  out |> Format.formatter_of_out_channel |> writer;
