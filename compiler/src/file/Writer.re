/**
 Utilities for writing to disk.
 */

let write = (out: out_channel, writer: Format.formatter => unit) =>
  out |> Format.formatter_of_out_channel |> writer;
