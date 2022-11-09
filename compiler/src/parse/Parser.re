open Knot.Kore;

type stream_t = LazyStream.t(Input.t);
type t('a) = stream_t => option(('a, stream_t));

/**
 Entrypoint for parsing a stream of unicode characters.
 */
let parse = (parser, input) =>
  parser(input) |> Option.map(((res, _)) => res);
