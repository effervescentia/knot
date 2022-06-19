/**
 Entrypoint for parsing a stream of unicode characters.
 */
let parse = (parser, input) =>
  parser(input) |> Option.map(((res, _)) => res);
